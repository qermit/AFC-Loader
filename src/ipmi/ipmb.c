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

#include <stdint.h>
#include <string.h>
#include "board.h"



#include "ipmb.h"
#include "ipmi.h"
#include "board_api.h"
#include <asf.h>


#if USE_FREERTOS
#warning "MMC Verion"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#else
#warning "BOOTLOADER Verion"
#endif

//static struct ipmi_msg * current_msg

#define SEND_TYPE_RESP 1
#define SEND_TYPE_EVENT 2

static int send_type = 0;

// @todo: add Request/Response recognition
int ipmb_decode(struct ipmi_msg *dst, uint8_t * buffer, int length) {
	uint8_t *data_ptr = buffer + 6;
	struct ipmi_ipmb_addr * src_addr = (struct ipmi_ipmb_addr *)&dst->saddr;
	struct ipmi_ipmb_addr * dst_addr = (struct ipmi_ipmb_addr *)&dst->daddr;

	if (length < 3) return -1;
	if (ipmb_crc(buffer, 3) != 0) return -2;
	if (ipmb_crc(buffer, length) != 0) return -3;


	dst->msg.netfn = (buffer[1] >> 2) & 0b00111111;
	dst->msg.cmd = buffer[5];

	src_addr->slave_addr = buffer[3];
	src_addr->lun = buffer[4] & 0x03;

	dst_addr->slave_addr = buffer[0];
	dst_addr->lun = buffer[1] & 0x03;
	dst->msg.lun = buffer[1] & 0x03;

	dst->sequence = (buffer[4] >> 2 ) & 0b00111111;

	// @ todo: sprawdzic
	dst->msg.data = dst->msg_data;
	dst->msg.data_len = length - 6 -1; // last one is crc

	int i = 0;
	if (dst->msg.netfn & 0x01) {
		// This is response;
		dst->retcode =data_ptr[0];
		data_ptr++;
	}
	for (i = 0 ; i<dst->msg.data_len; i++) {
		dst->msg_data[i] = data_ptr[i];
	}
	dst->msg.data = dst->msg_data;

	return 0;
}

// @todo: add Request/Response recognition
int ipmb_encode(uint8_t * buffer, struct ipmi_msg *pmsg, int length) {
	int i;
	pmsg->msg.data = pmsg->msg_data;
	uint8_t * d_buffer = &buffer[6];
	struct ipmi_ipmb_addr * dst_addr = (struct ipmi_ipmb_addr *) &pmsg->daddr;
	struct ipmi_ipmb_addr * src_addr = (struct ipmi_ipmb_addr *) &pmsg->saddr;
	buffer[0] = dst_addr->slave_addr;
	buffer[1] = (0b11111100 & (pmsg->msg.netfn << 2 )) | (0b00000011 & dst_addr->lun);
	buffer[2] = ipmb_crc(buffer, 2);
	buffer[3] = src_addr->slave_addr;
	buffer[4] = (0b11111100 & (pmsg->sequence << 2 )) | (0b00000011 & src_addr->lun);
	buffer[5] = pmsg->msg.cmd;

	if (pmsg->msg.netfn & 0x01) {
			// This is response;
		*d_buffer = pmsg->retcode;
		d_buffer++;
	}

	for (i = 0; i < pmsg->msg.data_len; i++, d_buffer++ ) {
		*d_buffer = pmsg->msg_data[i];
	}

	*d_buffer =  ipmb_crc(buffer, d_buffer - buffer);
	int out_length = d_buffer - buffer +1;
	return out_length;

}

uint8_t ipmb_crc(uint8_t *buffer, int length) {
	uint8_t crc = 0;
	uint8_t *current_buffer = buffer;
	uint8_t *last_buffer = buffer + length;
	while(current_buffer != last_buffer) {
		crc -= *current_buffer++;
	}

	return crc;

}

/* EEPROM SLAVE data */
#define I2C_SLAVE_EEPROM_SIZE       64
#define I2C_SLAVE_EEPROM_ADDR       0x3B
#define I2C_SLAVE_IOX_ADDR          0x5B
static I2C_XFER_T seep_xfer;
static uint8_t seep_data[I2C_SLAVE_EEPROM_SIZE + 1];
static uint8_t i2c_output_buffer[32 + 1];

/* Slave event handler for simulated EEPROM */

//#include "board_api.h"
//void Board_LED_Set(uint8_t LEDNumber, bool On);
//void Board_LED_Toggle(uint8_t LEDNumber);
//int IPMB_I2C_EventHandler_done = 0;

#if USE_FREERTOS == 1
extern SemaphoreHandle_t ipmi_message_sent_sid;


void IPMB_I2C_EventHandler(I2C_ID_T id, I2C_EVENT_T event)
{
	static BaseType_t xHigherPriorityTaskWoken;
	if (event == I2C_EVENT_LOCK) return;

	if (event == I2C_EVENT_DONE) {
		xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(ipmi_message_sent_sid, &xHigherPriorityTaskWoken);
		//portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
		 //ipmi_message_sent_sid
	}


}

#endif

void IPMB_send_event_done() {
	static BaseType_t xHigherPriorityTaskWoken;
	xHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR(ipmi_message_sent_sid, &xHigherPriorityTaskWoken);
}

TWI_Slave_t slave;
char readBuffer[24] ;
void IPMB_event_done() {
	struct ipmi_msg * p_ipmi_req = NULL;
	uint8_t decode_result = -1;
	uint8_t i;
	
	//if (slave.status != TWIS_STATUS_READY)  return;
	if (slave.result != TWIS_RESULT_OK)  return;
	p_ipmi_req = IPMI_alloc_fromISR();
	
	seep_data[0] = slave.slave_address;
	
	
	if (p_ipmi_req != NULL)
			decode_result = ipmb_decode(p_ipmi_req, seep_data, slave.bytesReceived+1);
	
	Board_LED_Toggle(LED_RED);		
		if (decode_result == 0) {
			if (p_ipmi_req->msg.netfn & 0x01) {
				// handle response, not expecting any response
				// @todo: event response handling for standard mmc code
				#ifdef FREERTOS_CONFIG_H
				//IPMI_free_fromISR(p_ipmi_req);
				IPMI_put_event_response(p_ipmi_req);
				#else
				IPMI_free_fromISR(p_ipmi_req);
				#endif
			} else {
				// handle request
				if (IPMI_req_queue_append_fromISR(p_ipmi_req)  != 0) {
					IPMI_free_fromISR(p_ipmi_req);
				} 
			}
		} else {
			IPMI_free_fromISR(p_ipmi_req);
		}

}

static void IPMB_events(I2C_ID_T id, I2C_EVENT_T event)
{
	uint8_t * ptr;
	int decode_result;
	struct ipmi_msg* p_ipmi_req;
//	Board_LED_Toggle(2);
	switch(event) {
		case I2C_EVENT_DONE:
			//DEBUGOUT_IPMB("%x %x %d\r\n", seep_data, seep_xfer.rxBuff, seep_xfer.rxSz);
			seep_data[0] = seep_xfer.slaveAddr;
			DEBUGOUT_IPMB("===\r\nIPMB IN:  ", seep_xfer.rxSz);
			for (ptr = seep_data; ptr < seep_xfer.rxBuff; ptr++) {
				DEBUGOUT_IPMB("%02x ", *ptr);
			}
			DEBUGOUT_IPMB("\r\n");
			decode_result = -1;
			p_ipmi_req = IPMI_alloc_fromISR();
			if (p_ipmi_req != NULL)
				decode_result = ipmb_decode(p_ipmi_req, seep_data, seep_xfer.rxBuff - seep_data);


			if (decode_result == 0) {
				if (p_ipmi_req->msg.netfn & 0x01) {
					// handle response, not expecting any response
					// @todo: event response handling for standard mmc code
					//IPMI_free_fromISR(p_ipmi_req);
					IPMI_put_event_response(p_ipmi_req);
				} else {
					// handle request
					if (IPMI_req_queue_append_fromISR(p_ipmi_req)  != 0) {
						IPMI_free_fromISR(p_ipmi_req);
					}
				}
			} else {
				IPMI_free_fromISR(p_ipmi_req);
			}

			//DEBUGOUT_IPMB("CRC: %02x, %02x, %02x, %02x\r\n",ipmb_crc(seep_data, 3), ipmb_crc(seep_data, 2),ipmb_crc(seep_data, seep_xfer.rxBuff - seep_data ),ipmb_crc(seep_data, seep_xfer.rxBuff - seep_data -1 ));
			seep_xfer.rxBuff = &seep_data[1];
			seep_xfer.rxSz = 32;
			break;

		case I2C_EVENT_SLAVE_RX:
			//if (seep_xfer.slaveAddr )
			//DEBUGOUT_IPMB("%02X ", seep_xfer.rxBuff);
			//pos++;
			break;


		case I2C_EVENT_SLAVE_TX:
			// Tego nie obslugujemy w ipmb
			seep_xfer.txSz = 0;
			break;
		default:
			break;
	}
}

ISR(TWIC_TWIS_vect,ISR_NAKED)
{
	portSAVE_CONTEXT();
	portSwitchToSystem();
	TWI_SlaveInterruptHandler(&slave);
	portRESTORE_CONTEXT();
	asm volatile ("reti");
}

ISR(TWIC_TWIM_vect,ISR_NAKED)
{
	portSAVE_CONTEXT();
	portSwitchToSystem();
	twim_interrupt_handler();
	portRESTORE_CONTEXT();
	asm volatile ("reti");
}


unsigned char IPMB_slave_addr;

/* Simulate an I2C EEPROM slave */
unsigned char IPMB_init(I2C_ID_T id)
{
	twi_options_t m_options;
	memset(seep_data, 0xFF, I2C_SLAVE_EEPROM_SIZE);
    sysclk_enable_peripheral_clock(&TWIC);
	
	IPMB_slave_addr = ipmb_get_GA();
	IPMB_slave_addr = 0x76;
	seep_xfer.slaveAddr = IPMB_slave_addr;

	seep_xfer.txBuff = &seep_data[1];
	seep_xfer.rxBuff = &seep_data[1];
	seep_xfer.txSz = seep_xfer.rxSz = sizeof(seep_data) - 1;
	seep_xfer.rxSz = 32;

	slave.receivedData = &seep_data[1];
	slave.slave_address = IPMB_slave_addr;

    TWI_SlaveInitializeDriver(&slave, &TWIC, *IPMB_event_done);
    TWI_SlaveInitializeModule(&slave, IPMB_slave_addr >> 1,TWI_SLAVE_INTLVL_MED_gc);

	m_options.speed     = 100000;
	m_options.chip      = IPMB_slave_addr >> 1;
	m_options.speed_reg = TWI_BAUD(sysclk_get_cpu_hz(), 100000);

	
	twi_master_init(&TWIC, &m_options);
	twi_master_enable(&TWIC);

	//Chip_I2C_SlaveSetup(id, I2C_SLAVE_0, &seep_xfer, IPMB_events, 0);
#ifdef FREERTOS_CONFIG_H
//	if (id == I2C0)
//		Chip_I2C_SetMasterEventHandler(id, IPMB_I2C_EventHandler);
    
#endif
	return IPMB_slave_addr;
}




I2C_XFER_T tmp_xfer = {0};
	
 twi_package_t tmp_xfer_packet;
 
void IPMB_send(struct ipmi_msg * msg) {
	int length  = ipmb_encode(i2c_output_buffer, msg, 32);
	int i;

	DEBUGOUT_IPMB("IPMB out: ");
	for (i = 0; i< length; i++) {
		DEBUGOUT_IPMB("%02x ", i2c_output_buffer[i]);
	}
	DEBUGOUT_IPMB("\r\n");

	//* todo - change to send and forget
//	Board_LED_Set(1,1);
/*
	tmp_xfer.slaveAddr = i2c_output_buffer[0] >> 1 ;
	tmp_xfer.txBuff = &i2c_output_buffer[1];
	tmp_xfer.txSz = length -1;
	tmp_xfer.rxSz = 0;
	*/

	tmp_xfer_packet.addr_length = 0;
	tmp_xfer_packet.chip = i2c_output_buffer[0] >> 1;
	tmp_xfer_packet.buffer = &i2c_output_buffer[1];
	tmp_xfer_packet.length = length - 1;
	tmp_xfer_packet.no_wait = true;
	tmp_xfer_packet.Process_Data = IPMB_send_event_done;
#ifdef FREERTOS_CONFIG_H
	twi_master_write(&TWIC, &tmp_xfer_packet);
	Board_LED_Toggle(LED_BLUE);
	//Chip_I2C_MasterTransferXfer(I2C0, &tmp_xfer);
#else
	//Chip_I2C_MasterTransfer(I2C0, &tmp_xfer);
#endif
//	Board_LED_Set(1,0);

}


/*
 *  based on afcipm/src/i2c.c
 *  author: Henrique Silva  <henrique.silva@lnls.br>
 */


uint8_t IPMBL_TABLE[] = {
    0x70, 0x8A, 0x72, 0x8E, 0x92, 0x90, 0x74, 0x8C, 0x76,
    0x98, 0x9C, 0x9A, 0xA0, 0xA4, 0x88, 0x9E, 0x86, 0x84,
    0x78, 0x94, 0x7A, 0x96, 0x82, 0x80, 0x7C, 0x7E, 0xA2 };

#define IPMBL_TABLE_SIZE (sizeof(IPMBL_TABLE) / sizeof(uint8_t))

uint8_t ipmb_get_slot_by_address(uint8_t address){
	if (address & 0x01) return 0;
	if (address < 0x70) return 0;

	uint8_t slot = 0x70 - address;
	return (slot / 2);

}

uint8_t ipmb_get_address_by_slot(uint8_t slot){
	if (slot >= IPMBL_TABLE_SIZE) return 0x70;
	return (0x70+(slot*2));
}


/*
 *  based on afcipm/src/i2c.c
 *  author: Henrique Silva  <henrique.silva@lnls.br>
 */

uint8_t ipmb_get_GA( void )
{
    uint8_t ga0, ga1, ga2;
    uint8_t index;
    uint8_t i;
    uint8_t address = IPMBL_TABLE[0];

    /* Clar the test pin and read all GA pins */
	#warning "spawdzic jak to zrobic"
	ioport_set_pin_mode(PIN_GA_TEST,  IOPORT_DIR_OUTPUT| IOPORT_INIT_HIGH);
	ioport_set_pin_high(PIN_GA_TEST);
	ioport_set_pin_mode(PIN_GA0,  IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PIN_GA1,  IOPORT_DIR_INPUT);
	ioport_set_pin_mode(PIN_GA2,  IOPORT_DIR_INPUT);


	ga0 = ioport_get_pin_level(PIN_GA0);
	ga1 = ioport_get_pin_level(PIN_GA1);
	ga2 = ioport_get_pin_level(PIN_GA2);
	
    /* Set the test pin and see if any GA pin has changed is value,
     * meaning that it is unconnected */
	ioport_set_pin_low(PIN_GA_TEST);
    for(i=0; i<100; i++); {asm("nop");}

    if ( ga0 != ioport_get_pin_level(PIN_GA0) )
    {
        ga0 = UNCONNECTED;
    }

    if ( ga1 != ioport_get_pin_level(PIN_GA1) )
    {
        ga1 = UNCONNECTED;
    }

    if ( ga2 != ioport_get_pin_level(PIN_GA2) )
    {
        ga2 = UNCONNECTED;
    }

    /* Transform the 3-based code in a decimal number */
    index = (9 * ga2) + (3 * ga1) + (1 * ga0);

    if ( index >= IPMBL_TABLE_SIZE )
    {
    	index = 0;
    }

    address = IPMBL_TABLE[index];
    return address;
}

