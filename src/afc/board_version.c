/*
 * board_version.c
 *
 *   AFCIPMI  --
 *
 *   Copyright (C) 2015  Piotr Miedzik <P.Miedzik@gsi.de>
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

#include "board.h"
#include "board_version.h"
#include "../ipmi/ipmb.h"
#if USE_FREERTOS == 1

#include <FreeRTOS.h>
#include <semphr.h>

#endif

static struct manufacturing_info_raw afc_board_info;




// Chip_ID
// 0 <- LM75AIM
// 1 <- LM75AIM
// 2 <- LM75AIM
// 3 <- LM75AIM
// 4 <- MAX6642ATT90
// 5 <- MCP
// 6 <- AT24MAC602

struct i2c_chip_mapping {
	uint8_t chip_id;
	uint8_t bus_id;
	uint8_t i2c_address;
	uint8_t i2c_address2;
};

struct i2c_bus_mapping {
	uint8_t bus_id;
	I2C_ID_T i2c_interface;
	int8_t mux_bus; // AFCv1, AFCv2 mux between P0[0],P0[1] and P0[19]/P[20]
					 // AFCv3 MUX IC79 on cpu id
	                 // AFCv3.1
	                 // -1 does not uses bus mux
	uint8_t enabled;
};

struct i2c_mux_state {
	I2C_ID_T i2c_interface;
	int8_t state;
#if USE_FREERTOS == 1
	SemaphoreHandle_t semaphore;
	TickType_t start_time;
#endif
};

struct i2c_mux_state i2c_mux[] = {
		{ I2C1, -1, 0, 0 },
		{ I2C2, -1, 0, 0 },
};

#define I2C_MUX_COUNT (sizeof(i2c_mux) / sizeof(struct i2c_mux_state))




struct i2c_bus_mapping *p_i2c_busmap = NULL;

struct i2c_bus_mapping i2c_bus_map_afc_v2[] = {
		{ I2C_BUS_FMC1_ID,  I2C1,  0, 1 },
		{ I2C_BUS_FMC2_ID,  I2C2, -1, 1 },
		{ I2C_BUS_CPU_ID,   I2C1,  1, 1 },
		{ I2C_BUS_RTM_ID,   I2C1,  0, 0 },
		{ I2C_BUS_CLOCK_ID, I2C1,  0, 1 },
		{ I2C_BUS_FPGA_ID,  I2C1,  0, 0 },
};

struct i2c_bus_mapping i2c_bus_map_afc_v3[] = {
		{ I2C_BUS_FMC1_ID,  I2C1,  0, 1 },
		{ I2C_BUS_FMC2_ID,  I2C1,  1, 1 },
		{ I2C_BUS_CPU_ID,   I2C1, -1, 1 },
		{ I2C_BUS_RTM_ID,   I2C1,  3, 1 },
		{ I2C_BUS_CLOCK_ID, I2C1,  2, 1 },
		{ I2C_BUS_FPGA_ID,  I2C1,  0, 0 },
};

struct i2c_bus_mapping i2c_bus_map_afc_v3_1[] = {
		{ I2C_BUS_FMC1_ID,  I2C2,  1, 1 },
		{ I2C_BUS_FMC2_ID,  I2C2,  0, 1 },
		{ I2C_BUS_CPU_ID,   I2C1, -1, 1 },
		{ I2C_BUS_RTM_ID,   I2C2,  3, 1 },
		{ I2C_BUS_CLOCK_ID, I2C2,  2, 1 },
		{ I2C_BUS_FPGA_ID,  I2C2, -1, 1 },
};




struct i2c_chip_mapping i2c_chip_map[] = {
//		{CHIP_ID_MUX      ,  I2C_BUS_CPU_ID, 0x70},
		{CHIP_ID_MUX      ,  I2C_BUS_CPU_ID, 0x70},
		{CHIP_ID_LM75AIM_0,  I2C_BUS_CPU_ID, 0x4C},
		{CHIP_ID_LM75AIM_1,  I2C_BUS_CPU_ID, 0x4D},
		{CHIP_ID_LM75AIM_2,  I2C_BUS_CPU_ID, 0x4E},
		{CHIP_ID_LM75AIM_3,  I2C_BUS_CPU_ID, 0x4F},
		{CHIP_ID_MAX6642,    I2C_BUS_CPU_ID, 0x48},

		{CHIP_ID_RTC,        I2C_BUS_CPU_ID, 0x9F},
		{CHIP_ID_RTC_EEPROM, I2C_BUS_CPU_ID, 0x57},
		{CHIP_ID_EEPROM,     I2C_BUS_CPU_ID, 0x50},
		{CHIP_ID_EEPROM_ID,  I2C_BUS_CPU_ID, 0x58},

		{CHIP_ID_INA_0,      I2C_BUS_CPU_ID, 0x40},
		{CHIP_ID_INA_1,      I2C_BUS_CPU_ID, 0x41},
		{CHIP_ID_INA_2,      I2C_BUS_CPU_ID, 0x42},
		{CHIP_ID_INA_3,      I2C_BUS_CPU_ID, 0x43},
		{CHIP_ID_INA_4,      I2C_BUS_CPU_ID, 0x44},
		{CHIP_ID_INA_5,      I2C_BUS_CPU_ID, 0x45},

		{CHIP_ID_ADN,	     I2C_BUS_CLOCK_ID, 0x4B},
		{CHIP_ID_SI57x,		 I2C_BUS_CLOCK_ID, 0x30},

		{CHIP_ID_EEPROM_FMC1,	     I2C_BUS_FMC1_ID, 0x4B},
		{CHIP_ID_EEPROM_FMC2,		 I2C_BUS_FMC2_ID, 0x30},

};

#define I2C_CHIP_MAP_COUNT (sizeof(i2c_chip_map)/sizeof(struct i2c_chip_mapping))


/* Set I2C mode to polling/interrupt */
static void i2c_set_mode(I2C_ID_T id, int polling)
{
	IRQn_Type irq_type = I2C0_IRQn;
	switch (id) {
		case I2C0: irq_type = I2C0_IRQn; break ;
		case I2C1: irq_type = I2C1_IRQn; break ;
		case I2C2: irq_type = I2C2_IRQn; break ;
		default: return;

	}

	if(!polling) {
		//mode_poll &= ~(1 << id);
		Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandler);
		NVIC_EnableIRQ(irq_type);
	} else {
		//mode_poll |= 1 << id;
		NVIC_DisableIRQ(irq_type);
		Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandlerPolling);
	}
}


void i2c_app_init(I2C_ID_T id, int speed, int pooling)
{
	Board_I2C_Init(id);

	/* Initialize I2C */
	Chip_I2C_Init(id);
	Chip_I2C_SetClockRate(id, speed);

	/* Set default mode to interrupt */
	i2c_set_mode(id, pooling);
}

void afc_board_i2c_init() {
	int i;
	for (i=0; i<I2C_MUX_COUNT; i++) {
		i2c_mux[i].semaphore = xSemaphoreCreateBinary();
		//i2c_mux[i].i2c_interface = I2C1;
		i2c_app_init(i2c_mux[i].i2c_interface, SPEED_100KHZ, I2CMODE_INTERRUPT);
		xSemaphoreGive(i2c_mux[i].semaphore);

	}
}

void afc_board_discover()
{
	//@todo: discover board revision
	unsigned char tx_params[1];
	tx_params[0] = 0xF0 ; // Protected eeprom address

	I2C_XFER_T xfer = { 0 };
	xfer.slaveAddr = 0x57; // RTC EEPROM
	xfer.txBuff = tx_params;
	xfer.txSz = 1;
	xfer.rxBuff = (uint8_t *) &afc_board_info;
	xfer.rxSz = sizeof(afc_board_info);

	while (Chip_I2C_MasterTransfer(I2C1, &xfer) == I2C_STATUS_ARBLOST) {
	}

	uint8_t crc_fail = ipmb_crc((uint8_t *) &afc_board_info, 8 );

	if (crc_fail == 0) {
		if ((afc_board_info.carrier_type == CARRIER_TYPE_AFC && afc_board_info.board_version == 0x00 )||
			(afc_board_info.carrier_type == CARRIER_TYPE_AFC && afc_board_info.board_version == 0x01)) {

			i2c_chip_map[CHIP_ID_MUX].bus_id = I2C_BUS_UNKNOWN_ID;
			i2c_chip_map[CHIP_ID_MUX].i2c_address = 0x00;
			p_i2c_busmap = i2c_bus_map_afc_v2;
		} else if ((afc_board_info.carrier_type == CARRIER_TYPE_AFC && afc_board_info.board_version == 0x02) ||
				afc_board_info.carrier_type == CARRIER_TYPE_AFCK) {
			p_i2c_busmap = i2c_bus_map_afc_v3;
			i2c_chip_map[CHIP_ID_MUX].bus_id = I2C_BUS_CPU_ID;
			i2c_chip_map[CHIP_ID_MUX].i2c_address = 0x70;
		} else if ((afc_board_info.carrier_type == CARRIER_TYPE_AFC && afc_board_info.board_version == 0x02)) {
			p_i2c_busmap = i2c_bus_map_afc_v3_1;
			i2c_chip_map[CHIP_ID_MUX].bus_id = I2C_BUS_FPGA_ID;
			i2c_chip_map[CHIP_ID_MUX].i2c_address = 0x70;
		}

		asm("nop");
	} else {
		DEBUGOUT("BoardManufactured CRC fail\r\n");



//	DEBUGOUT("carrier type: 0x%02X\r\n", afc_board_info.carrier_version);
	//@todo: discover i2c layout if fai;
	}
}

void afc_get_manufacturing_info(struct manufacturing_info_raw *p_board_info)
{

}

void afc_get_board_type(uint8_t *carrier_type, uint8_t *board_version) {
	if (carrier_type != NULL)
		*carrier_type = afc_board_info.carrier_type;

	if (board_version != NULL)
		*board_version = afc_board_info.board_version;
}



Bool afc_i2c_take_by_busid(uint8_t bus_id, I2C_ID_T * i2c_interface, TickType_t max_wait_time) {

	int i;
	struct i2c_mux_state * p_i2c_mux = NULL;
	struct i2c_bus_mapping * p_i2c_bus = &p_i2c_busmap[bus_id];

	I2C_ID_T tmp_interface_id = p_i2c_busmap[bus_id].i2c_interface;
	for (i=0; i<I2C_MUX_COUNT; i++) {
		if (i2c_mux[i].i2c_interface == tmp_interface_id) {
			p_i2c_mux = &i2c_mux[i];
			break;
		}
	}

	if (p_i2c_mux == NULL) return false;
	if (p_i2c_bus->enabled == 0) return false;
	if (p_i2c_mux->semaphore == 0) return false;

	if (xSemaphoreTake(p_i2c_mux->semaphore, max_wait_time) == pdFALSE) {
		return false;
	}

	if (p_i2c_bus->mux_bus == -1) {
		// this bus is not multiplexed
		*i2c_interface = p_i2c_mux->i2c_interface;
		return true;
	}
	if (p_i2c_mux->state == p_i2c_bus->mux_bus) {
		// this bus mux is in correct state
		*i2c_interface = p_i2c_mux->i2c_interface;
		return true;
	} else if (i2c_chip_map[CHIP_ID_MUX].bus_id == I2C_BUS_UNKNOWN_ID) {

		if (p_i2c_bus->mux_bus == 0) {
			Chip_I2C_Disable(p_i2c_bus->i2c_interface);
			Chip_I2C_DeInit(p_i2c_bus->i2c_interface);

			Chip_IOCON_PinMux(LPC_IOCON, 0, 0, IOCON_MODE_INACT, IOCON_FUNC0);
			Chip_IOCON_PinMux(LPC_IOCON, 0, 1, IOCON_MODE_INACT, IOCON_FUNC0);
			//Chip_IOCON_EnableOD(LPC_IOCON, 0,  0);
			//Chip_IOCON_EnableOD(LPC_IOCON, 0,  1);

			Chip_IOCON_PinMux(LPC_IOCON, 0, 19, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_PinMux(LPC_IOCON, 0, 20, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_EnableOD(LPC_IOCON, 0, 19);
			Chip_IOCON_EnableOD(LPC_IOCON, 0, 20);
			Chip_I2C_Init(p_i2c_bus->i2c_interface);
			Chip_I2C_Enable(p_i2c_bus->i2c_interface);

		} else if ((p_i2c_bus->mux_bus == 1)) {
			Chip_I2C_Disable(p_i2c_bus->i2c_interface);

			Chip_I2C_DeInit(p_i2c_bus->i2c_interface);

			Chip_IOCON_PinMux(LPC_IOCON, 0, 0, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_PinMux(LPC_IOCON, 0, 1, IOCON_MODE_INACT, IOCON_FUNC3);
			Chip_IOCON_EnableOD(LPC_IOCON, 0, 0);
			Chip_IOCON_EnableOD(LPC_IOCON, 0, 1);

			Chip_IOCON_PinMux(LPC_IOCON, 0, 19, IOCON_MODE_INACT, IOCON_FUNC0);
			Chip_IOCON_PinMux(LPC_IOCON, 0, 20, IOCON_MODE_INACT, IOCON_FUNC0);
			//Chip_IOCON_EnableOD(LPC_IOCON, 0, 19);
			//Chip_IOCON_EnableOD(LPC_IOCON, 0, 20);

			Chip_I2C_Init(p_i2c_bus->i2c_interface);
			Chip_I2C_Enable(p_i2c_bus->i2c_interface);

		} else {
			xSemaphoreGive(p_i2c_mux->semaphore);
			return false;
		}
		p_i2c_mux->state = p_i2c_bus->mux_bus;
		*i2c_interface = p_i2c_mux->i2c_interface;
		return true;
		// this is mux inside MMC
	} else {
		I2C_XFER_T xfer = {
			 .slaveAddr = i2c_chip_map[CHIP_ID_MUX].i2c_address,
			 .txSz = 1,
			 .txBuff = &p_i2c_bus->mux_bus,
			 .rxSz = 0,
			 .rxBuff = NULL,
		};
		while (Chip_I2C_MasterTransfer(i2c_chip_map[CHIP_ID_MUX].bus_id, &xfer) == I2C_STATUS_ARBLOST) {
		}
		p_i2c_mux->state = p_i2c_bus->mux_bus;
		*i2c_interface = p_i2c_mux->i2c_interface;
		return true;
	}


	xSemaphoreGive(p_i2c_mux->semaphore);
	return false;

}

Bool afc_i2c_take_by_chipid(uint8_t chip_id, uint8_t * i2c_address, I2C_ID_T * i2c_interface,  TickType_t max_wait_time) {
	if (chip_id > I2C_CHIP_MAP_COUNT) return false;

	uint8_t bus_id = i2c_chip_map[chip_id].bus_id;
	if (i2c_address != NULL)
		*i2c_address = i2c_chip_map[chip_id].i2c_address;

	return afc_i2c_take_by_busid(bus_id, i2c_interface, max_wait_time);
}

void afc_i2c_give(I2C_ID_T i2c_interface) {

	int i;
	for (i=0; i<I2C_MUX_COUNT; i++) {
		if (i2c_mux[i].i2c_interface == i2c_interface) {
			xSemaphoreGive(i2c_mux[i].semaphore);
			break;
		}
	}
}
