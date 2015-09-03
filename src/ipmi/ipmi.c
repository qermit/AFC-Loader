/*
 * ipmi.c
 *
 *   AFCIPMI  --
 *
 *   Copyright (C) 2015  Piotr Miedzik  <P.Miedzik@gsi.de>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ipmi.h"
#include "ipmi_handlers.h"
#include "ipmi_oem.h"
#include "sdr.h"
#include <string.h>

#if USE_FREERTOS == 1
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#endif



static const ipmiFuncEntry_t const ipmiEntries[] = {
		{ NETFN_APP,     IPMI_GET_DEVICE_ID_CMD, ipmi_get_device_id},
		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_GET_PROPERTIES, ipmi_picmg_get_PROPERTIES},
		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_FRU_CONTROL, ipmi_picmg_cmd_fru_control},
		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_SET_FRU_LED_STATE, ipmi_picmg_set_fru_led_state},
		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_GET_DEVICE_LOCATOR_RECORD, ipmi_picmg_get_device_locator_record},
		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_SET_AMC_PORT_STATE, ipmi_picmg_cmd_set_amc_port_state },
		//{ NETFN_GRPEXT,  IPMI_PICMG_CMD_GET_TELCO_ALARM_CAPABILITY, ipmi_picmg_cmd_get_telco_alarm_capability},
		{ NETFN_SE,      IPMI_SET_EVENT_RECEIVER_CMD, ipmi_se_set_event_reciever},
		{ NETFN_SE,      IPMI_GET_DEVICE_SDR_INFO_CMD, ipmi_se_get_sdr_info},
		{ NETFN_SE,      IPMI_GET_DEVICE_SDR_CMD, ipmi_se_get_sdr},
		{ NETFN_SE,		 IPMI_GET_SENSOR_READING_CMD, ipmi_se_get_sensor_reading},
		{ NETFN_SE,      IPMI_RESERVE_DEVICE_SDR_REPOSITORY_CMD, ipmi_se_reserve_device_sdr},

		{ NETFN_STORAGE, IPMI_GET_FRU_INVENTORY_AREA_INFO_CMD, ipmi_storage_get_fru_info},
		{ NETFN_STORAGE, IPMI_READ_FRU_DATA_CMD, ipmi_storage_read_fru_data_cmd},
		{ NETFN_CUSTOM_AFC, IPMI_AFC_CMD_I2C_TRANSFER, ipmi_afc_i2c_transfer},
		{ NETFN_CUSTOM_AFC, IPMI_AFC_CMD_GPIO, ipmi_afc_gpio},
		{ NETFN_CUSTOM_AFC, IPMI_AFC_CMD_SSP_TRANSFER, ipmi_afc_ssp_transfer},
		{ NETFN_CUSTOM_AFC, IPMI_AFC_CMD_SSP_TRANSFER_RAW, ipmi_afc_ssp_transfer_raw},
		{ 0, 0, NULL }
};

// @todo: moze jakis mutex
struct ipmi_ipmb_addr event_src;
struct ipmi_ipmb_addr event_dst;

void IPMI_evet_set_address(struct ipmi_ipmb_addr * src, struct ipmi_ipmb_addr * dst) {
	if (src == NULL ) {
		memset(&event_src,0, sizeof(event_src));
	} else {
		memcpy(&event_src, src, sizeof(event_src));
	}

	if ( dst == NULL) {
		memset(&event_dst,0, sizeof(event_dst));
	} else {
		memcpy(&event_dst, dst, sizeof(event_dst));
	}
}

void IPMI_evet_get_address(struct ipmi_ipmb_addr * src, struct ipmi_ipmb_addr * dst) {
	if (src!= NULL) {
		memcpy(src, &event_src, sizeof(event_src));
	}
	if (dst!= NULL) {
		memcpy(dst, &event_dst, sizeof(event_dst));
	}
}

#ifdef FREERTOS_CONFIG_H
#define IPMI_BUFF_COUNT 10
#else
#define IPMI_BUFF_COUNT 2
#endif

struct ipmi_msg ipmi_buff[IPMI_BUFF_COUNT];
volatile static unsigned char   ipmi_buff_alloc[IPMI_BUFF_COUNT] = { 0, 0};

volatile static struct ipmi_msg * pending_req_msg = NULL;
volatile static int pending_req = 0;
#ifdef FREERTOS_CONFIG_H
static QueueHandle_t free_msg_queue;
static QueueHandle_t req_msg_queue;
static QueueHandle_t event_queue_id;
SemaphoreHandle_t ipmi_message_sent_sid;
SemaphoreHandle_t ipmi_message_recvd_sid;
#define IPMI_STATE_IDLE 0
#define IPMI_STATE_SEND_RESP 1
#define IPMI_STATE_SEND_EVENT 2
#define IPMI_STATE_RECV_REQ 3
#define IPMI_STATE_RECV_EVENT 4
#define IPMI_STATE_WAITING_EVENT 5


struct ipmi_msg * event_sent_waiting = NULL;
struct ipmi_msg * event_recv = NULL;
int _ipmi_state_req = 0;
int _ipmi_state_event = 0;
#endif

static struct ipmi_msg * pending_resp_msg = NULL;


void IPMI_init(){
	int i;
	pending_req = 0;
	pending_resp_msg = NULL;
	for (i=0;i<IPMI_BUFF_COUNT; i++) {
		ipmi_buff_alloc[i] = 0;
		memset(&ipmi_buff[i], 0, sizeof(struct ipmi_msg));

	}

#ifdef FREERTOS_CONFIG_H
	free_msg_queue = xQueueCreate(IPMI_BUFF_COUNT, sizeof(struct ipmi_msg*));
	req_msg_queue = xQueueCreate(IPMI_BUFF_COUNT, sizeof(struct ipmi_msg*));
	event_queue_id = xQueueCreate(IPMI_BUFF_COUNT, sizeof(struct ipmi_msg*));
	ipmi_message_sent_sid = xSemaphoreCreateBinary();
	ipmi_message_recvd_sid = xSemaphoreCreateBinary();
	for (i=0;i<IPMI_BUFF_COUNT; i++) {
		struct ipmi_msg * ptr = &ipmi_buff[i];
		xQueueSend(free_msg_queue, &ptr, ( TickType_t ) 0);
	}

#endif
}

#ifdef FREERTOS_CONFIG_H

struct ipmi_msg * IPMI_alloc_fromISR() {

	int i;
//	DEBUGOUT("IPMI_alloc_isr[%d] ", _ipmi_state);
	struct ipmi_msg * retval = NULL;
	if (xQueueReceiveFromISR(free_msg_queue, &retval , NULL) == pdTRUE ) {
//		DEBUGOUT("%x\r\n", retval);
		return retval;
	} else {
		DEBUGOUT("IPMI allocisr fail\r\n");
		return NULL;

	}
}
#endif

struct ipmi_msg * IPMI_alloc() {
	struct ipmi_msg * retval = NULL;
#ifdef FREERTOS_CONFIG_H

//	DEBUGOUT("IPMI_alloc ");
	if (xQueueReceive(free_msg_queue, &retval , ( TickType_t ) 0) == pdTRUE ) {
//		DEBUGOUT("%x\r\n", retval);
		retval->retries_left = 3;
		return retval;
	} else {
		DEBUGOUT("IPMI alloc fail\r\n");
		return NULL;
	}
#else
	int i;

	for (i = 0; i<IPMI_BUFF_COUNT; i++) {
		if (ipmi_buff_alloc[i] == 0) {
			ipmi_buff_alloc[i] = 1;
			DEBUGOUT("IPMI alloc: %d\r\n", i);
			return &ipmi_buff[i];
		}
	}
	DEBUGOUT("IPMI_alloc fail\r\n");
	return NULL;
#endif
}


#ifdef FREERTOS_CONFIG_H
void IPMI_free_fromISR(struct ipmi_msg * msg) {
	if (msg < ipmi_buff) return;
	xQueueSendFromISR(free_msg_queue, &msg, NULL);
	return;
}
#else
//#define IPMI_free_fromISR(_msg) IPMI_free(_msg)
#endif


void IPMI_free(struct ipmi_msg * msg) {
	if (msg < ipmi_buff) return;
#ifdef FREERTOS_CONFIG_H

	xQueueSend(free_msg_queue, &msg, (TickType_t) 0);
	return;
#else
	int index = (msg - ipmi_buff);
	//index = index / sizeof(struct ipmi_msg);
	if (index > IPMI_BUFF_COUNT) return;
	ipmi_buff_alloc[index] = 0;
#endif
}

#ifdef FREERTOS_CONFIG_H
int IPMI_req_queue_append_fromISR(struct ipmi_msg * msg) {

	if (msg == NULL) {
		return -1;
	}
	xQueueSendFromISR(req_msg_queue, &msg, NULL);
	//xSemaphoreGiveFromISR(ipmi_message_recvd_sid, NULL);

	return 0;
}
#else
//#define IPMI_req_queue_append_fromISR(_msg) IPMI_req_queue_append(_msg)
#endif

int IPMI_req_queue_append(struct ipmi_msg * msg) {
#ifdef FREERTOS_CONFIG_H

	if (msg == NULL) {
		return -1;
	}

	xQueueSend(req_msg_queue, &msg, (TickType_t) 0);
	return 0;
#else
	if (msg == NULL) {
		return -1;
	} else if (pending_req_msg != NULL) {
		return -2;
	}

	pending_req++;
	pending_req_msg = msg;
	return 0;
#endif
}

int IPMI_event_queue_append(struct ipmi_msg * msg) {
#ifdef FREERTOS_CONFIG_H

	if (msg == NULL) {
		return -1;
	}

	xQueueSend(event_queue_id, &msg, (TickType_t) 0);
	return 0;
#else
#error Not implemented yet
	if (msg == NULL) {
		return -1;
	} else if (pending_req_msg != NULL) {
		return -2;
	}

	pending_req++;
	pending_req_msg = msg;
	return 0;
#endif
}

void IPMI_req_queue_pushback(struct ipmi_msg * msg) {
#ifdef FREERTOS_CONFIG_H

	if (msg == NULL) {
			return;
		}
	xQueueSendToFront(req_msg_queue, &msg, (TickType_t) 0);
	return;
#else


	pending_req++;
	pending_req_msg = msg;
	if (pending_req_msg == NULL) pending_req--;
#endif
}

struct ipmi_msg * IPMI_req_queue_get(  int wait_time ) {
	struct ipmi_msg * retval = NULL;

#ifdef FREERTOS_CONFIG_H
	if (xQueueReceive(req_msg_queue, &retval , (TickType_t) wait_time) == pdTRUE ) {
		return retval;
	}
	return NULL;
#else
	if (pending_req) {
		pending_req--;
		retval = pending_req_msg;
		pending_req_msg = NULL;
	}
	return retval;
#endif
}

void IPMI_send_resp(struct ipmi_msg *msg) {

//#ifdef FREERTOS_CONFIG_H


//#else
	if (pending_resp_msg != NULL) {
		IPMI_free(pending_resp_msg);
	}
	pending_resp_msg = msg;

//	IPMB_send(p_ipmi_resp);
//	IPMI_free(p_ipmi_resp);
//#endif
}


void IPMI_send_proc() {
	//@todo: lock muttex
	if (pending_resp_msg == NULL) return;

	struct ipmi_msg * tmp_msg = pending_resp_msg;
	pending_resp_msg = NULL;

	//@todo: release mutex

	IPMB_send(tmp_msg);

	IPMI_free(tmp_msg);


}


void IPMI_check_req() {
	// tutaj starac sie pobrac
	struct ipmi_msg * p_ipmi_req ;
	struct ipmi_msg * p_ipmi_req_next1 = NULL;
	struct ipmi_msg * p_ipmi_req_next2 ;
	struct ipmi_msg * p_ipmi_resp ;

	struct ipmi_ipmb_addr * tmp_dst;
	struct ipmi_ipmb_addr * tmp_src;
#ifdef FREERTOS_CONFIG_H
	p_ipmi_req = IPMI_req_queue_get( 0 );
#else
	p_ipmi_req = IPMI_req_queue_get( -1 );
#endif
	if (p_ipmi_req == NULL) return;

	unsigned char error = IPMI_CC_OK;

	// tutaj ipmi_malloc()
	p_ipmi_resp = IPMI_alloc();

	if (p_ipmi_resp == NULL) {
		IPMI_req_queue_pushback(p_ipmi_req);
		return;
	}

	p_ipmi_resp->msg.data_len = 0;


	ipmiFuncEntry_t * p_ptr = (ipmiFuncEntry_t *) ipmiEntries;
	while (p_ptr->process != NULL) {
	    	//DEBUGOUT("Function: %d:%d => %x\r\n", p_ptr->netfn, p_ptr->cmd, p_ptr->process);
		if(p_ptr->netfn == p_ipmi_req->msg.netfn && p_ptr->cmd == p_ipmi_req->msg.cmd) {
			break;
		}
	    p_ptr++;
	}
	if (p_ptr->process == NULL) {
		ipmi_general_invalid(p_ipmi_req, p_ipmi_resp);
		//p_ipmi_resp->retcode = IPMI_CC_INV_CMD;
	} else {
		tmp_src = & p_ipmi_req->saddr;
		//tmp_src->lun = 0;
		tmp_dst = & p_ipmi_req->daddr;

		p_ptr->process(p_ipmi_req, p_ipmi_resp);

	}

	error = p_ipmi_resp->retcode;
	DEBUGOUT("IPMI netfn/cmd: %02x/%02x -> %02x\r\n",p_ipmi_req->msg.netfn, p_ipmi_req->msg.cmd, error);

	memcpy(&p_ipmi_resp->daddr, &p_ipmi_req->saddr, sizeof(struct ipmi_addr));
	memcpy(&p_ipmi_resp->saddr, &p_ipmi_req->daddr, sizeof(struct ipmi_addr));
	p_ipmi_resp->msg.cmd   = p_ipmi_req->msg.cmd;
	p_ipmi_resp->msg.netfn = p_ipmi_req->msg.netfn +1;
	p_ipmi_resp->sequence  = p_ipmi_req->sequence;
	//p_ipmi_resp->msg_data[0] = error;

	// @todo: zrobic przekopywanie sie do ostatniego requestu
#ifdef FREERTOS_CONFIG_H
	while (uxQueueMessagesWaiting(req_msg_queue)) {
		IPMI_free(p_ipmi_req_next1);
		p_ipmi_req_next1 = IPMI_req_queue_get(0);
	}
		if (p_ipmi_req_next1 != NULL ) {
			//DEBUGOUT("Nex message\n")
			if (p_ipmi_req_next1->sequence == p_ipmi_resp->sequence) {
				IPMI_free(p_ipmi_req_next1);
			} else {
			//	DEBUGOUT("Discarding message\r\n");
				IPMI_free(p_ipmi_req);
				IPMI_free(p_ipmi_resp);
				IPMI_req_queue_pushback(p_ipmi_req_next1);
				return;
			}
		}

#endif

	// Tutaj ipmi push to queue;
	IPMI_free(p_ipmi_req);
	IPMI_send_resp(p_ipmi_resp);
//	IPMB_send(p_ipmi_resp);
//	IPMI_free(p_ipmi_resp);


}

#ifdef FREERTOS_CONFIG_H

void IPMI_put_event_response(struct ipmi_msg * p_ipmi_req) {
	event_recv = p_ipmi_req;
	static BaseType_t xHigherPriorityTaskWoken;
	xSemaphoreGiveFromISR(ipmi_message_recvd_sid, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}


void vTaskIPMI( void *pvParmeters )
{
	TickType_t xLastWakeTime, xCurrentWakeTime;
	QueueSetMemberHandle_t xActivatedMember;
	int counts[3] = { 0, 0 ,0};

	QueueSetHandle_t event_set = xQueueCreateSet(IPMI_BUFF_COUNT*2+2);
	xQueueAddToSet(req_msg_queue, event_set);
	xQueueAddToSet(event_queue_id, event_set);
	xQueueAddToSet(ipmi_message_sent_sid, event_set);
	xQueueAddToSet(ipmi_message_recvd_sid, event_set);
	xLastWakeTime = xTaskGetTickCount();
	_ipmi_state_event = 0;
	_ipmi_state_req = 0;

	for( ;; )
	{
		// Wait for the next cycle.
		//vTaskDelayUntil( &xLastWakeTime, 1 );
		xActivatedMember = xQueueSelectFromSet( event_set, 1000 / portTICK_PERIOD_MS ); // timeout na 1 sekunde
		xCurrentWakeTime = xTaskGetTickCount();
		if (xActivatedMember == req_msg_queue) {
			IPMI_check_req();
			_ipmi_state_req = IPMI_STATE_SEND_RESP;
			IPMI_send_proc();
			continue;
		} else if (xActivatedMember == ipmi_message_recvd_sid) {
			xSemaphoreTake(xActivatedMember, 0);
			if (_ipmi_state_event == IPMI_STATE_WAITING_EVENT) {
				IPMI_free(event_sent_waiting); event_sent_waiting = 0;
				IPMI_free(event_recv); event_recv = 0;
				_ipmi_state_event = IPMI_STATE_IDLE;
				continue;
			} else {
				// otrzymano wiadomosc, ale juz obsluzono ja chyba
			}
		} else if (xActivatedMember == ipmi_message_sent_sid) {
			xSemaphoreTake(xActivatedMember, 0);
			if (_ipmi_state_req == IPMI_STATE_SEND_RESP) {
				_ipmi_state_req = IPMI_STATE_IDLE;
			}
			if (_ipmi_state_event == IPMI_STATE_SEND_EVENT ) {
				_ipmi_state_event = IPMI_STATE_WAITING_EVENT;
				xLastWakeTime = xTaskGetTickCount();
			}
		} else if (xActivatedMember == event_queue_id) {
			if (_ipmi_state_event == IPMI_STATE_IDLE && _ipmi_state_req != IPMI_STATE_SEND_RESP) {
				xQueueReceive(event_queue_id, &event_sent_waiting , 0);
				IPMB_send(event_sent_waiting);
				_ipmi_state_event = IPMI_STATE_SEND_EVENT;
			}
		}

//		counts[0] = uxQueueMessagesWaiting(req_msg_queue);
//		counts[1] = uxQueueMessagesWaiting(event_queue_id);
//		counts[2] = uxQueueMessagesWaiting(free_msg_queue);
//		DEBUGOUT("MEM stat = %d %d %d\r\n", counts[0], counts[1], counts[2]);

		if (_ipmi_state_req == IPMI_STATE_IDLE  && _ipmi_state_event != IPMI_STATE_SEND_EVENT) {
			if (counts[0] + counts[1] + counts[2] < IPMI_BUFF_COUNT-2) {
				asm("nop");
			}
			if (uxQueueMessagesWaiting(req_msg_queue) != 0) {
				IPMI_check_req();
				_ipmi_state_req = IPMI_STATE_SEND_RESP;
				IPMI_send_proc();
			}
		}
		if (_ipmi_state_event == IPMI_STATE_WAITING_EVENT) {

			TickType_t dif =  getTickDifference(xCurrentWakeTime, xLastWakeTime) * portTICK_PERIOD_MS;
			if (dif > 1000) {

				if (event_sent_waiting->retries_left-- > 0) {
					xQueueSendToFront(event_queue_id, &event_sent_waiting, 0);
				} else {
//					DEBUGOUT("REMOVING event\r\n");
					IPMI_free(event_sent_waiting);
					event_sent_waiting=NULL;
				}
				_ipmi_state_event = IPMI_STATE_IDLE;
			}
			//if (getTickDifference(TickType_t current_time, TickType_t start_time) *  )

			//DEBUGOUT("IPMI_STATE_WAITING_EVENT = %d - %d = %d\r\n", xCurrentWakeTime, xLastWakeTime, dif);
//xCurrentWakeTime - xLastWakeTime);
		}
		if (_ipmi_state_event == IPMI_STATE_IDLE  && _ipmi_state_req != IPMI_STATE_SEND_RESP) {
			if (counts[0] + counts[1] + counts[2] < IPMI_BUFF_COUNT) {
				asm("nop");
			}
			if (uxQueueMessagesWaiting(event_queue_id) != 0){
				xQueueReceive(event_queue_id, &event_sent_waiting , 0);
				_ipmi_state_event = IPMI_STATE_SEND_EVENT;
				IPMB_send(event_sent_waiting);
			}

		}
	}
}
#endif
