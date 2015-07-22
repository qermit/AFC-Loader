/*
 * ipmb.h
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


#ifndef IPMB_H_
#define IPMB_H_

#include <stdint.h>
#include "ipmi.h"
#include "board.h"

// zwraca kod bledu, jezeli 0 to poszlo wszhystko ok
int ipmb_decode(struct ipmi_msg *dst, uint8_t * buffer, int length);
// Zwraca dlugosc bajtow
int ipmb_encode(uint8_t * buffer, struct ipmi_msg *dst, int length);

// jezeli suma kontrolna ok, to zwraca 0
// jezeli suma kontrolna nie ok, to zwraca oczekiwana sume kontrolna
// moze byc uzyta do tworzenia sumy kontrolnej
uint8_t ipmb_crc(uint8_t *buffer, int length);

void IPMB_init(I2C_ID_T id);


void IPMB_send(struct ipmi_msg * msg);

#endif /* IPMB_H_ */
