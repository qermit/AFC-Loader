/*
 * ipmi_oem.c
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


#include "FreeRTOS.h"

#include "ipmi_handlers.h"
#include "ipmi.h"
#include "board_api.h"
#include "i2c_17xx_40xx.h"

#include "task.h"
#include "semphr.h"

#include "ipmi_oem.h"

struct I2C_Mutex i2c_mutex_array[2];

void ipmi_afc_gpio(struct ipmi_msg *req, struct ipmi_msg* rsp) {
	uint8_t port = req->msg_data[0];
	uint8_t mode = req->msg_data[1];

	if (mode == 0) {
		// no change, get port status
		uint32_t *ret_val = (uint32_t *)(&rsp->msg_data[0]);
		ret_val[0] = Chip_GPIO_GetPortDIR(LPC_GPIO, port);
		ret_val[1] = Chip_GPIO_GetPortValue(LPC_GPIO, port);
		rsp->msg.data_len=8;
	} else {
		uint8_t pin = req->msg_data[2];
		if (mode == 1) {
			// input
			Chip_GPIO_SetPinDIRInput(LPC_GPIO, port, pin);
		} else if (mode == 2) {
			// pin output, set mode
			Chip_GPIO_SetPinDIROutput(LPC_GPIO, port, pin);
			if (req->msg.data_len == 4) {
				uint8_t value = req->msg_data[3];
				Chip_GPIO_SetPinState(LPC_GPIO, port, pin, value);
			}
		}
		rsp->msg_data[0] = Chip_GPIO_GetPinDIR(LPC_GPIO, port, pin);
		rsp->msg_data[1] = Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
		rsp->msg.data_len=2;
	}

	rsp->retcode = IPMI_CC_OK;

}

void ipmi_afc_i2c_transfer(struct ipmi_msg *req, struct ipmi_msg* rsp)
{
	uint8_t i2c_bus = req->msg_data[0];
	int i2c_bus_id = 0;
	switch (i2c_bus) {
		case 0:
			i2c_bus_id= 0; // I2C_FMC1
			break;
		case 1:
			i2c_bus_id= 1; // I2C_FMC2
			break;
		case 2:
			i2c_bus_id= 0; // I2C_CPU
			break;
		default:
			i2c_bus_id= 0;
	}

	uint8_t i2c_bytes_sent = 0;
	uint8_t i2c_bytes_recv = 0;


	I2C_XFER_T xfer = {0};
	xfer.slaveAddr = req->msg_data[1];
	xfer.txBuff = &req->msg_data[4];
	xfer.txSz = req->msg_data[2];
	xfer.rxBuff = &rsp->msg_data[2];
	xfer.rxSz = req->msg_data[3];


	SemaphoreHandle_t xSemaphore  = i2c_mutex_array[i2c_bus_id].semaphore;
	if (xSemaphoreTake(xSemaphore, (TickType_t)100) == pdTRUE) {
		// reconfigure bus
		if (i2c_bus_id == 0 && i2c_bus == 2) {
			Chip_I2C_Disable(i2c_mutex_array[i2c_bus_id].i2c_bus);

			Chip_I2C_DeInit(i2c_mutex_array[i2c_bus_id].i2c_bus);

			Chip_IOCON_PinMux(LPC_IOCON, 0,  0, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_PinMux(LPC_IOCON, 0,  1, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_EnableOD(LPC_IOCON, 0,  0);
			Chip_IOCON_EnableOD(LPC_IOCON, 0,  1);

			Chip_IOCON_PinMux(LPC_IOCON, 0, 19, IOCON_MODE_INACT, IOCON_FUNC0);
			Chip_IOCON_PinMux(LPC_IOCON, 0, 20, IOCON_MODE_INACT, IOCON_FUNC0);
		//Chip_IOCON_EnableOD(LPC_IOCON, 0, 19);
		//Chip_IOCON_EnableOD(LPC_IOCON, 0, 20);

			Chip_I2C_Init(i2c_mutex_array[i2c_bus_id].i2c_bus);
			Chip_I2C_Enable(i2c_mutex_array[i2c_bus_id].i2c_bus);
		}

		i2c_mutex_array[i2c_bus_id].start_time = xTaskGetTickCount();
		while (Chip_I2C_MasterTransfer(i2c_mutex_array[i2c_bus_id].i2c_bus, &xfer) == I2C_STATUS_ARBLOST) {
		}
		i2c_bytes_sent = req->msg_data[2] - xfer.txSz;
		i2c_bytes_recv = req->msg_data[3] - xfer.rxSz;

		if (i2c_bus_id == 0 && i2c_bus == 2) {
			Chip_I2C_Disable(i2c_mutex_array[i2c_bus_id].i2c_bus);

			Chip_I2C_DeInit(i2c_mutex_array[i2c_bus_id].i2c_bus);

			Chip_IOCON_PinMux(LPC_IOCON, 0,  0, IOCON_MODE_INACT, IOCON_FUNC0);
			Chip_IOCON_PinMux(LPC_IOCON, 0,  1, IOCON_MODE_INACT, IOCON_FUNC0);
			//Chip_IOCON_EnableOD(LPC_IOCON, 0,  0);
			//Chip_IOCON_EnableOD(LPC_IOCON, 0,  1);

			Chip_IOCON_PinMux(LPC_IOCON, 0, 19, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_PinMux(LPC_IOCON, 0, 20, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_EnableOD(LPC_IOCON, 0, 19);
			Chip_IOCON_EnableOD(LPC_IOCON, 0, 20);
			Chip_I2C_Init(i2c_mutex_array[i2c_bus_id].i2c_bus);
			Chip_I2C_Enable(i2c_mutex_array[i2c_bus_id].i2c_bus);
		}


		xSemaphoreGive(i2c_mutex_array[i2c_bus_id].semaphore);

	} else {
		asm("nop");
	}

	rsp->msg_data[0] = i2c_bytes_sent;
	rsp->msg_data[1] = i2c_bytes_recv;
	rsp->msg.data_len = 2 + i2c_bytes_recv;
	rsp->retcode = IPMI_CC_OK;

}
