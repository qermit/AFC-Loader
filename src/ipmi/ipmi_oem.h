/*
 * ipmi_oem.h
 *
 *  Created on: 14 sie 2015
 *      Author: pmiedzik
 */

#ifndef IPMI_IPMI_OEM_H_
#define IPMI_IPMI_OEM_H_

#include "board.h"
#include "semphr.h"

#define IPMI_AFC_CMD_I2C_TRANSFER			0x00
#define IPMI_AFC_CMD_CLOCK_CROSSBAR_GET		0x02
#define IPMI_AFC_CMD_CLOCK_CROSSBAR_SET		0x03

struct I2C_Mutex {
	SemaphoreHandle_t semaphore;
	TickType_t start_time;
	I2C_ID_T i2c_bus;
};


ipmiProcessFunc ipmi_afc_i2c_transfer(struct ipmi_msg *req, struct ipmi_msg* rsp);


#endif /* IPMI_IPMI_OEM_H_ */
