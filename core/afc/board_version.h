/*
 * board_version.h
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

#ifndef AFC_BOARD_VERSION_H_
#define AFC_BOARD_VERSION_H_

#include <stdint.h>
#include <FreeRTOS.h>

#define CARRIER_TYPE_UNKNOWN 0xFF
#define CARRIER_TYPE_AFC 0x01
#define CARRIER_TYPE_AFCK 0x02

#define BOARD_VERSION_UNKNOWN 0xFF


#define I2CMODE_POOLING 1
#define I2CMODE_INTERRUPT 0
#define SPEED_100KHZ         100000

// BUS_ID
// 0 - FMC1
// 1 - FMC2
// 3 - CPU_ID
//
///////////////////////
#define I2C_BUS_UNKNOWN_ID  255
#define I2C_BUS_FMC1_ID  0
#define I2C_BUS_FMC2_ID  1
#define I2C_BUS_CPU_ID   2
#define I2C_BUS_RTM_ID   3
#define I2C_BUS_CLOCK_ID 4
#define I2C_BUS_FPGA_ID  5

#define CHIP_ID_MUX       0
#define CHIP_ID_LM75AIM_0 1
#define CHIP_ID_LM75AIM_1 2
#define CHIP_ID_LM75AIM_2 3
#define CHIP_ID_LM75AIM_3 4
#define CHIP_ID_MAX6642 5

#define CHIP_ID_RTC 6
#define CHIP_ID_RTC_EEPROM 7
#define CHIP_ID_EEPROM 8
#define CHIP_ID_EEPROM_ID 9
#define CHIP_ID_EEPROM_PSWP 10

#define CHIP_ID_INA_0 11
#define CHIP_ID_INA_1 12
#define CHIP_ID_INA_2 13
#define CHIP_ID_INA_3 14
#define CHIP_ID_INA_4 15
#define CHIP_ID_INA_5 16

#define CHIP_ID_ADN 17
#define CHIP_ID_SI57x 18

#define CHIP_ID_EEPROM_FMC1 19
#define CHIP_ID_EEPROM_FMC2 20



struct manufacturing_info_raw {
	unsigned char carrier_type;
	unsigned char board_version;
	unsigned char manufacturer[3];
	unsigned char manufacturing_day[2];
	unsigned char crc;
};

struct manufacturing_info {
	uint8_t carrier_version;
	uint8_t board_version;
	uint32_t manufacturer;
	uint16_t manufacturing_day;
};

void i2c_app_init(I2C_ID_T id, int speed, int pooling);
void afc_board_i2c_init();
void afc_board_discover();
void afc_get_manufacturing_info(struct manufacturing_info_raw *p_board_info);
void afc_get_board_type(uint8_t *carrier_type, uint8_t *board_version);
Bool afc_i2c_take_by_busid(uint8_t bus_id, I2C_ID_T * i2c_interface, TickType_t max_wait_time);
Bool afc_i2c_take_by_chipid(uint8_t chip_id, uint8_t * i2c_address, I2C_ID_T * i2c_interface,  TickType_t max_wait_time) ;
void afc_i2c_give(I2C_ID_T i2c_interface);


#endif /* AFC_BOARD_VERSION_H_ */
