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
#include "sdr.h"
#include <string.h>



static const ipmiFuncEntry_t const ipmiEntries[] = {
		{ NETFN_APP,     IPMI_GET_DEVICE_ID_CMD, ipmi_get_device_id},
		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_GET_PROPERTIES, ipmi_picmg_get_PROPERTIES},
		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_SET_FRU_LED_STATE, ipmi_picmg_set_fru_led_state},
		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_GET_DEVICE_LOCATOR_RECORD, ipmi_picmg_get_device_locator_record},
		{ NETFN_SE,      IPMI_SET_EVENT_RECEIVER_CMD, ipmi_se_set_event_reciever},
		{ NETFN_SE,      IPMI_GET_DEVICE_SDR_INFO_CMD, ipmi_se_get_sdr_info},
		{ NETFN_SE,      IPMI_GET_DEVICE_SDR_CMD, ipmi_se_get_sdr},
		{ NETFN_SE,		 IPMI_GET_SENSOR_READING_CMD, ipmi_se_get_sensor_reading},
		{ NETFN_SE,      IPMI_RESERVE_DEVICE_SDR_REPOSITORY_CMD, ipmi_se_reserve_device_sdr},
		{ NETFN_STORAGE, IPMI_GET_FRU_INVENTORY_AREA_INFO_CMD, ipmi_storage_get_fru_info},
		{ NETFN_STORAGE, IPMI_READ_FRU_DATA_CMD, ipmi_storage_read_fru_data_cmd},
		{ 0, 0, NULL }
};

#define IPMI_BUFF_COUNT 2
struct ipmi_msg ipmi_buff[IPMI_BUFF_COUNT];
static unsigned char   ipmi_buff_alloc[IPMI_BUFF_COUNT] = { 0, 0};

static struct ipmi_msg * pending_req_msg = NULL;
static int pending_req;

struct ipmi_msg * IPMI_alloc() {
	int i;
	for (i = 0; i<IPMI_BUFF_COUNT; i++) {
		if (ipmi_buff_alloc[i] == 0) {
			ipmi_buff_alloc[i] = 1;
			return &ipmi_buff[i];
		}
	}
	DEBUGOUT("IPMI_alloc fail\r\n");
	return NULL;
}

void IPMI_free(struct ipmi_msg * msg) {
	if (msg < ipmi_buff) return;
	int index = (msg - ipmi_buff);
	//index = index / sizeof(struct ipmi_msg);
	if (index > IPMI_BUFF_COUNT) return;
	ipmi_buff_alloc[index] = 0;
}

int IPMI_req_queue_append(struct ipmi_msg * msg) {
	if (msg == NULL) {
		return -1;
	} else if (pending_req_msg != NULL) {
		return -2;
	}

	pending_req = 1;
	pending_req_msg = msg;
	return 0;
}

void IPMI_req_queue_pushback(struct ipmi_msg * msg) {
	pending_req = 1;
	pending_req_msg = msg;
	if (pending_req_msg == NULL) pending_req = 0;

}

struct ipmi_msg * IPMI_req_queue_get() {
	pending_req = 0;
	struct ipmi_msg * retval = pending_req_msg;
	pending_req_msg = NULL;
	return retval;
}


void IPMI_check_req() {
	// tutaj starac sie pobrac
	struct ipmi_msg * p_ipmi_req ;
	struct ipmi_msg * p_ipmi_req_next1 ;
	struct ipmi_msg * p_ipmi_req_next2 ;
	struct ipmi_msg * p_ipmi_resp ;
	p_ipmi_req = IPMI_req_queue_get();
	if (p_ipmi_req == NULL) return;

	unsigned char error = IPMI_CC_OK;

	// tutaj ipmi_malloc()
	p_ipmi_resp = IPMI_alloc();

	if (p_ipmi_resp == NULL) {
		IPMI_req_queue_pushback(p_ipmi_req);
		return;
	}

	p_ipmi_resp->msg.data_len = 1;

	ipmiFuncEntry_t * p_ptr = (ipmiFuncEntry_t *) ipmiEntries;
	while (p_ptr->process != NULL) {
	    	//DEBUGOUT("Function: %d:%d => %x\r\n", p_ptr->netfn, p_ptr->cmd, p_ptr->process);
		if(p_ptr->netfn == p_ipmi_req->msg.netfn && p_ptr->cmd == p_ipmi_req->msg.cmd) {
			break;
		}
	    p_ptr++;
	}
	if (p_ptr->process == NULL) {
		error = IPMI_CC_INV_CMD;
	} else {
		p_ptr->process(p_ipmi_req, p_ipmi_resp);
		error = p_ipmi_resp->retcode;
	}

	DEBUGOUT("IPMI netfn/cmd: %02x/%02x -> %02x\r\n",p_ipmi_req->msg.netfn, p_ipmi_req->msg.cmd, error);

	memcpy(&p_ipmi_resp->daddr, &p_ipmi_req->saddr, sizeof(struct ipmi_addr));
	memcpy(&p_ipmi_resp->saddr, &p_ipmi_req->daddr, sizeof(struct ipmi_addr));
	p_ipmi_resp->msg.cmd   = p_ipmi_req->msg.cmd;
	p_ipmi_resp->msg.netfn = p_ipmi_req->msg.netfn +1;
	p_ipmi_resp->sequence  = p_ipmi_req->sequence;
	p_ipmi_resp->msg_data[0] = error;

	// @todo: zrobic przekopywanie sie do ostatniego requestu
	p_ipmi_req_next1 = IPMI_req_queue_get();
	if (p_ipmi_req_next1 != NULL ) {
		if (p_ipmi_req_next1->sequence == p_ipmi_resp->sequence) {
			IPMI_free(p_ipmi_req_next1);
		} else {
			DEBUGOUT("Discarding message\r\n");
			IPMI_req_queue_pushback(p_ipmi_req_next1);
			IPMI_free(p_ipmi_req);
			IPMI_free(p_ipmi_resp);
			return;
		}
	}

	// Tutaj ipmi push to queue;
	IPMB_send(p_ipmi_resp);
	IPMI_free(p_ipmi_resp);
	IPMI_free(p_ipmi_req);

}

