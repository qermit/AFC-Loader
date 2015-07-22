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
#include "board.h"

#include "ipmb.h"
#include "ipmi.h"
#include "board_api.h"



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
	dst->msg.data_len = length - 6;
	int i;
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
	buffer[1] = (0b11111100 & (pmsg->msg.netfn << 2 )) | (0b00000011 & pmsg->msg.lun);
	buffer[2] = ipmb_crc(buffer, 2);
	buffer[3] = src_addr->slave_addr;
	buffer[4] = (0b11111100 & (pmsg->sequence << 2 )) | (0b00000011 & pmsg->msg.lun);
	buffer[5] = pmsg->msg.cmd;

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

static void IPMB_events(I2C_ID_T id, I2C_EVENT_T event)
{
	uint8_t * ptr;
	int decode_result;
	struct ipmi_msg* p_ipmi_req;
	switch(event) {
		case I2C_EVENT_DONE:
			//DEBUGOUT("%x %x %d\r\n", seep_data, seep_xfer.rxBuff, seep_xfer.rxSz);
			seep_data[0] = seep_xfer.slaveAddr;
			DEBUGOUT("===\r\nIPMB IN:  ", seep_xfer.rxSz);
			for (ptr = seep_data; ptr < seep_xfer.rxBuff; ptr++) {
				DEBUGOUT("%02x ", *ptr);
			}
			DEBUGOUT("\r\n");
			decode_result = -1;
			p_ipmi_req = IPMI_alloc();
			if (p_ipmi_req != NULL)
				decode_result = ipmb_decode(p_ipmi_req, seep_data, seep_xfer.rxBuff - seep_data);

			if (decode_result == 0) {
				if (p_ipmi_req->msg.netfn & 0x01) {
					// handle response, not expecting any response
					// @todo: event response handling for standard mmc code
					IPMI_free(p_ipmi_req);
				} else {
					// handle request
					if (IPMI_req_queue_append(p_ipmi_req)  != 0) {
						IPMI_free(p_ipmi_req);
					}
				}
			}

			//DEBUGOUT("CRC: %02x, %02x, %02x, %02x\r\n",ipmb_crc(seep_data, 3), ipmb_crc(seep_data, 2),ipmb_crc(seep_data, seep_xfer.rxBuff - seep_data ),ipmb_crc(seep_data, seep_xfer.rxBuff - seep_data -1 ));
			seep_xfer.rxBuff = &seep_data[1];
			seep_xfer.rxSz = 32;
			break;

		case I2C_EVENT_SLAVE_RX:
			//if (seep_xfer.slaveAddr )
			//DEBUGOUT("%02X ", seep_xfer.rxBuff);
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

/* Simulate an I2C EEPROM slave */
void IPMB_init(I2C_ID_T id)
{
	memset(seep_data, 0xFF, I2C_SLAVE_EEPROM_SIZE);
	seep_xfer.slaveAddr = (I2C_SLAVE_EEPROM_ADDR << 1);

	seep_xfer.txBuff = &seep_data[1];
	seep_xfer.rxBuff = &seep_data[1];
	seep_xfer.txSz = seep_xfer.rxSz = sizeof(seep_data) - 1;
	seep_xfer.rxSz = 32;
	Chip_I2C_SlaveSetup(id, I2C_SLAVE_0, &seep_xfer, IPMB_events, 0);
}

void IPMB_send(struct ipmi_msg * msg) {
	int length  = ipmb_encode(i2c_output_buffer, msg, 32);
	int i;
	DEBUGOUT("IPMB out: ");
	for (i = 0; i< length; i++) {
		DEBUGOUT("%02x ", i2c_output_buffer[i]);
	}
	DEBUGOUT("\r\n");
	Chip_I2C_MasterSend(I2C0, i2c_output_buffer[0] >> 1, &i2c_output_buffer[1], length -1);

}

