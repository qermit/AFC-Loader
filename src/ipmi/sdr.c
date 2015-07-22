/*
 * sdr.c
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


#include "sdr.h"
#define NUM_SENSOR 		1	/* Number of sensors */
#define NUM_SDR         2   /* Number of SDRs */

#define HOT_SWAP_SENSOR     0
#define HOT_SWAP        0xF2

//module handle sensor status
#define HOT_SWAP_CLOSED   0x00
#define HOT_SWAP_OPENED   0x01
#define HOT_SWAP_QUIESCED 0x02

/* Device Locator Record */
unsigned char SDR0[] = {
	0x00,	/* record number, LSB - filled by sdr_init() */
	0x00,	/* record number, MSB - filled by sdr_init() */
	0x51,	/* IPMI protocol version */
	0x12,	/* record type: device locator record */
	0x15,	/* record length - filled by sdr_init() */

	/* record key bytes */
	0x00,
	0x00,
	0x00,
	0x29,
	0x00,
	0x00,
	0x00,
	0xc1,
	0x00,
	0x00,
	0xcc,	/* 8 bit ASCII, number of bytes */
	'A','F','C',' ','L','O','A','D','E','R'};

/* Hot-Swap sensor */
unsigned char SDR1[] = {
	/* sensor record header */
/*1*/	0x03,			/* record number, LSB  - filled by SDR_Init()*/
	0x00,			/* record number, MSB  - filled by SDR_Init()*/
	0x51,			/* IPMI protocol version */
	0x01,			/* record type: full sensor */
	0x37,			/* record length: remaining bytes -> SDR_Init */

	/* record key bytes */
	0x00,			/* i2c address, -> SDR_Init */
	0x00,			/* sensor owner LUN */
/*8*/	HOT_SWAP_SENSOR,	/* sensor number */

	/* record body bytes */
	0xc1,			/* entity id: AMC Module */
	0x00,			/* entity instance -> SDR_Init */
	0x03,			/* init: event generation + scanning enabled */
	0xc1,			/* capabilities: auto re-arm,*/
	HOT_SWAP,			/* sensor type: HOT SWAP*/
	0x6f,			/* sensor reading*/
	0x07,			/* LSB assert event mask: 3 bit value */
/*16*/	0x00,			/* MSB assert event mask */
	0x07,			/* LSB deassert event mask: 3 bit value */
	0x00,			/* MSB deassert event mask */
	0x00,			/* LSB: readabled Threshold mask: all thresholds are readabled:  */
	0x00,			/* MSB: setabled Threshold mask: all thresholds are setabled: */
	0xc0,			/* sensor units 1 :*/
	0x00,			/* sensor units 2 :*/
	0x00,			/* sensor units 3 :*/
/*24*/	0x00,			/* Linearization */
	0x00,			/* M */
	0x00,			/* M - Tolerance */
	0x00,			/* B */
	0x00,			/* B - Accuracy */
	0x00,			/* Sensor direction */
	0x00,			/* R-Exp , B-Exp */
	0x00,			/* Analogue characteristics flags */
/*32*/	0x00,			/* Nominal reading */
	0x00,			/* Normal maximum */
	0x00,			/* Normal minimum */
	0x00,			/* Sensor Maximum reading */
	0x00,			/* Sensor Minimum reading */
	0x00,			/* Upper non-recoverable Threshold */
	0x00,			/* Upper critical Threshold */
	0x00,			/* Upper non critical Threshold */
/*40*/	0x00,			/* Lower non-recoverable Threshold */
	0x00,			/* Lower critical Threshold */
	0x00,			/* Lower non-critical Threshold */
	0x00,			/* positive going Threshold hysteresis value */
	0x00,			/* negative going Threshold hysteresis value */
	0x00,			/* reserved */
	0x00,			/* reserved */
	0x00, 			/* OEM reserved */
/*48*/	0xcc,			/* 8 bit ASCII, number of bytes */
	'F','R','U',' ','H','O','T','_','S','W','A','P'	/* sensor string */
};

unsigned char *sdrPtr[NUM_SDR] = {SDR0,SDR1};
unsigned char sdrLen[NUM_SDR] = {
	sizeof(SDR0),
	sizeof(SDR1)
};
static unsigned short reservationID;


ipmiProcessFunc ipmi_se_get_sdr_info(struct ipmi_msg *req, struct ipmi_msg* rsp)
{
	int len = rsp->msg.data_len;

	if (req->msg.data_len == 0 || req->msg_data[0] == 0) {
		rsp->msg_data[len++] = NUM_SENSOR;
	} else {
		rsp->msg_data[len++] = NUM_SDR;
	}
	rsp->msg_data[len++] = 0x81;
	rsp->msg.data_len = len;
    rsp->retcode = IPMI_CC_OK;
}

struct __attribute__((__packed__)) ipmi_se_get_sdr_param {
	unsigned char reservation_id[2];
	unsigned char record_id[2];
	unsigned char offset;
	unsigned char size;

} ipmi_se_get_sdr_param_t;

ipmiProcessFunc ipmi_se_get_sdr(struct ipmi_msg *req, struct ipmi_msg* rsp) {
	struct ipmi_se_get_sdr_param * params;
	params = (struct ipmi_se_get_sdr_param *) req->msg_data;
	//unsigned short test = params->
	unsigned short record_id = params->record_id[0] | (params->record_id[1] << 8);
	unsigned short reservation_id = params->reservation_id[0] | (params->reservation_id[1] << 8);
	int len = rsp->msg.data_len;
	int i;

	// @todo: add reservation check

	if (record_id >= NUM_SDR || (params->offset + params->size) > sdrLen[record_id]) {
		return IPMI_CC_REQ_DATA_NOT_PRESENT;
	} else if (record_id == NUM_SDR -1 ) {
		rsp->msg_data[len++] = 0xFF;
		rsp->msg_data[len++] = 0xFF;
	} else {
		rsp->msg_data[len++] = (record_id+1) & 0xff;	/* next record ID */
		rsp->msg_data[len++] = (record_id+1) >> 8;	/* next record ID */
	}

	for (i=0; i< params->size; i++) {
		rsp->msg_data[len++] = sdrPtr[record_id][i+params->offset];
	}

	rsp->msg.data_len = len;
    rsp->retcode = IPMI_CC_OK;

}


ipmiProcessFunc ipmi_se_reserve_device_sdr(struct ipmi_msg *req, struct ipmi_msg* rsp)
{
	int len = rsp->msg.data_len;

    reservationID++;
     if (reservationID == 0)
         reservationID = 1;
     rsp->msg_data[len++] = reservationID & 0xff;
     rsp->msg_data[len++] = reservationID >> 8;

     rsp->msg.data_len = len;
     rsp->retcode = IPMI_CC_OK;

}

ipmiProcessFunc ipmi_se_get_sensor_reading(struct ipmi_msg *req, struct ipmi_msg* rsp) {
	int sensor_number = req->msg_data[0];
	int len = rsp->msg.data_len;

	if (sensor_number == HOT_SWAP_SENSOR) {
		rsp->msg_data[len++] = HOT_SWAP_OPENED;
	}
    rsp->msg.data_len = len;
    rsp->retcode = IPMI_CC_OK;


}


