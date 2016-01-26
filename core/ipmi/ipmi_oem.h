/*
 * ipmi_oem.h
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

#ifndef IPMI_IPMI_OEM_H_
#define IPMI_IPMI_OEM_H_

#ifdef CONFIG_FREERTOS
#include "board.h"
#include "semphr.h"
#endif

#define IPMI_AFC_CMD_I2C_TRANSFER			0x00
#define IPMI_AFC_CMD_GPIO					0x01
#define IPMI_AFC_CMD_SSP_TRANSFER			0x04
#define IPMI_AFC_CMD_SSP_TRANSFER_RAW		0x05
#define IPMI_AFC_CMD_CLOCK_CROSSBAR_GET		0x02
#define IPMI_AFC_CMD_CLOCK_CROSSBAR_SET		0x03

struct I2C_Mutex {
#ifdef CONFIG_FREERTOS
	SemaphoreHandle_t semaphore;
	TickType_t start_time;
#endif
	I2C_ID_T i2c_bus;
};

void create_ssp1_mutex();

void ipmi_afc_gpio(struct ipmi_msg *req, struct ipmi_msg* rsp);
void ipmi_afc_i2c_transfer(struct ipmi_msg *req, struct ipmi_msg* rsp);
void ipmi_afc_ssp_transfer(struct ipmi_msg *req, struct ipmi_msg* rsp);
void ipmi_afc_ssp_transfer_raw(struct ipmi_msg *req, struct ipmi_msg* rsp);


#endif /* IPMI_IPMI_OEM_H_ */
