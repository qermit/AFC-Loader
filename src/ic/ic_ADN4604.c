/*
 * ic_ADN4604.c
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

#include "ic_ADN4604.h"

void adn4604_setup(I2C_ID_T i2c_bus) {
	I2C_XFER_T xfer = { 0 };
	uint8_t tx_buf[20];
	uint8_t rx_buf[20];
	int i = 0;
	tx_buf[i++] = 0x90;
	tx_buf[i++] = 0xef;
	tx_buf[i++] = 0xcd;
	tx_buf[i++] = 0x8D;
	tx_buf[i++] = 0xdd;
	tx_buf[i++] = 0xdd;
	tx_buf[i++] = 0xdd;
	tx_buf[i++] = 0xdd;
	tx_buf[i++] = 0x01;

	xfer.slaveAddr = 0x4B;
	xfer.txBuff = tx_buf;
	xfer.txSz = i;
	xfer.rxBuff = rx_buf;
	xfer.rxSz = 0;

	while (Chip_I2C_MasterTransfer(i2c_bus, &xfer) == I2C_STATUS_ARBLOST) {
	}


	// Select update map 0
	i = 0;
	tx_buf[i++] = 0x81;
	tx_buf[i++] = 0x00;

	xfer.slaveAddr = 0x4B;
	xfer.txBuff = tx_buf;
	xfer.txSz = i;
	xfer.rxBuff = rx_buf;
	xfer.rxSz = 0;
	while (Chip_I2C_MasterTransfer(i2c_bus, &xfer) == I2C_STATUS_ARBLOST) {
	}
	// perform update
	i = 0;
	tx_buf[i++] = 0x80;
	tx_buf[i++] = 0x01;

	xfer.slaveAddr = 0x4B;
	xfer.txBuff = tx_buf;
	xfer.txSz = i;
	xfer.rxBuff = rx_buf;
	xfer.rxSz = 0;
	while (Chip_I2C_MasterTransfer(i2c_bus, &xfer) == I2C_STATUS_ARBLOST) {
	}

	i = 0;
	tx_buf[i++] = 0x20;

	tx_buf[i++] = 0x00;
	tx_buf[i++] = 0x00;
	tx_buf[i++] = 0x00;
	tx_buf[i++] = 0x00;

	tx_buf[i++] = 0x30;
	tx_buf[i++] = 0x30;
	tx_buf[i++] = 0x30;
	tx_buf[i++] = 0x30;

	tx_buf[i++] = 0x30;
	tx_buf[i++] = 0x30;
	tx_buf[i++] = 0x30;
	tx_buf[i++] = 0x30;

	tx_buf[i++] = 0x00;
	tx_buf[i++] = 0x00;
	tx_buf[i++] = 0x30;
	tx_buf[i++] = 0x30;

	xfer.slaveAddr = 0x4B;
	xfer.txBuff = tx_buf;
	xfer.txSz = i;
	xfer.rxBuff = rx_buf;
	xfer.rxSz = 0;

	while (Chip_I2C_MasterTransfer(i2c_bus, &xfer) == I2C_STATUS_ARBLOST) {
	}
}

//	I2C_XFER_T xfer = {0};
//	xfer.slaveAddr = req->msg_data[1];
//	xfer.txBuff = &req->msg_data[4];
//	xfer.txSz = req->msg_data[2];
//	xfer.rxBuff = &rsp->msg_data[2];
//	xfer.rxSz = req->msg_data[3];
