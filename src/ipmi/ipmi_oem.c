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
#include "ssp_17xx_40xx.h"

#include "task.h"
#include "semphr.h"

#include "ipmi_oem.h"
#include <string.h>

struct I2C_Mutex i2c_mutex_array[2];
SemaphoreHandle_t ssp1_mutex;
static Chip_SSP_DATA_SETUP_T xf_setup;

static uint8_t ssp_buffer[256];
static uint32_t ssp_page= 0xffffffff ; //

static void ctrlSPI_CS( bool value);


void SSP1_IRQHandler(void) {
 asm("nop");
}

void EINT2_IRQHandler(void){
	//LPC_GPIO[0].DIR |= 1UL << 6;
	LPC_GPIO[0].CLR = 1UL << 6;
	LPC_GPIO[0].SET = 1UL << 6;
	//LPC_GPIO[0].DIR &= ~(1UL << 6);
	LPC_GPIO[0].DIR |= 1UL << 6;

	LPC_SYSCON->EXTINT |= 1UL << 2;
	NVIC_ClearPendingIRQ(EINT2_IRQn);
}

bool ssp_load_page(int32_t address) {
	if ((ssp_page & 0xffffff00) != (address & 0xffffff00) || 1) {
		SemaphoreHandle_t xSemaphore  = ssp1_mutex;
		ssp_page = address & 0x00ffff00;
		uint8_t tx_ssp_buffer[4];
		tx_ssp_buffer[0] = 0x03;
		tx_ssp_buffer[1] = (address >> 16) & 0x00FF;
		tx_ssp_buffer[2] = (address >> 8) & 0x00FF;
		tx_ssp_buffer[3] = 0x00;
		xf_setup.tx_data = tx_ssp_buffer;
		xf_setup.rx_data = NULL;
		xf_setup.tx_cnt = 0;
		xf_setup.rx_cnt = 0;
		xf_setup.length = 4;

		if (xSemaphoreTake(xSemaphore, (TickType_t)100) == pdTRUE) {
			Chip_SSP_SetMaster(LPC_SSP1, true);
			ctrlSPI_CS(true);

			// write command
			xf_setup.tx_data = tx_ssp_buffer;
			xf_setup.rx_data = NULL;
			xf_setup.tx_cnt = 0;
			xf_setup.rx_cnt = 0;
			xf_setup.length = 4;

			Chip_SSP_RWFrames_Blocking(LPC_SSP1, & xf_setup);


			// read part
			xf_setup.tx_data = NULL;
			xf_setup.rx_data = ssp_buffer;
			xf_setup.tx_cnt = 0;
			xf_setup.rx_cnt = 0;
			xf_setup.length = 256;

			Chip_SSP_RWFrames_Blocking(LPC_SSP1, & xf_setup);

			ctrlSPI_CS(false);
			xSemaphoreGive(xSemaphore);
		} else {
			return false;
		}
	}


}

void create_ssp1_mutex() {
	ssp1_mutex = xSemaphoreCreateBinary();
	xSemaphoreGive(ssp1_mutex);
}



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

static void ctrlSPI_CS( bool value) {
	if (value) {
		Chip_GPIO_SetPinDIR(LPC_GPIO, 0, 6, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, false);
	} else {
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, true);
		Chip_GPIO_SetPinDIR(LPC_GPIO, 0, 6, false);
	}
}

void ipmi_afc_ssp_transfer(struct ipmi_msg *req, struct ipmi_msg* rsp)
{

	uint8_t command = req->msg_data[0];
	if (command == 0x02) { // program page
		rsp->retcode = IPMI_CC_OK;
	} else if (command == 0x03) {
		uint8_t address_parts[3];
		uint32_t address;

		address_parts[0] = req->msg_data[1];
		address_parts[1] = req->msg_data[2];
		address_parts[2] = req->msg_data[3];
		address = (address_parts[0] << 16) | (address_parts[1] << 8) | (address_parts[2]);
		uint16_t length = 256 - address_parts[2];
		if (length > 16) length = 16;
		ssp_load_page(address);

		memcpy(rsp->msg_data, &ssp_buffer[address_parts[2]], length);
		rsp->msg.data_len = length;
		rsp->retcode = IPMI_CC_OK;
		return;
	}
	rsp->retcode = IPMI_CC_OUT_OF_SPACE;
	return;

	uint8_t write_length = req->msg_data[0];
	uint8_t read_length = req->msg_data[1];


	SemaphoreHandle_t xSemaphore  = ssp1_mutex;
	if (xSemaphoreTake(xSemaphore, (TickType_t)100) == pdTRUE) {
		Chip_SSP_SetMaster(LPC_SSP1, true);
		xf_setup.tx_data = &req->msg_data[2];
		xf_setup.rx_data = &rsp->msg_data[2];
		xf_setup.tx_cnt = 0;
		xf_setup.rx_cnt = 0;
		xf_setup.length = write_length + read_length;
		ctrlSPI_CS(true);
		Chip_SSP_RWFrames_Blocking(LPC_SSP1, & xf_setup);
		ctrlSPI_CS(false);


		xSemaphoreGive(xSemaphore);
	} else {
		asm("nop");
	}

	rsp->msg_data[0] = xf_setup.tx_cnt;
	rsp->msg_data[1] = xf_setup.rx_cnt;
	rsp->msg.data_len = 2 + write_length + read_length;
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
		if (i2c_bus_id == 0 && i2c_bus == 0) {
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

		i2c_mutex_array[i2c_bus_id].start_time = xTaskGetTickCount();
		while (Chip_I2C_MasterTransfer(i2c_mutex_array[i2c_bus_id].i2c_bus, &xfer) == I2C_STATUS_ARBLOST) {
		}
		i2c_bytes_sent = req->msg_data[2] - xfer.txSz;
		i2c_bytes_recv = req->msg_data[3] - xfer.rxSz;

		if (i2c_bus_id == 0 && i2c_bus == 0) {
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


		xSemaphoreGive(i2c_mutex_array[i2c_bus_id].semaphore);

	} else {
		asm("nop");
	}

	rsp->msg_data[0] = i2c_bytes_sent;
	rsp->msg_data[1] = i2c_bytes_recv;
	rsp->msg.data_len = 2 + i2c_bytes_recv;
	rsp->retcode = IPMI_CC_OK;

}
