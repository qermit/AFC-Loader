/*
 * fru.c
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

#include "fru.h"
#include "ipmi.h"


const char default_fru[8] = { 0x01, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xFF };
/*

 const fru_common_header_t default_fru = {
		.format_version = 0x01,
		.int_use_offset = 0,
		.chassis_info_offset = 0,
		.board_offset = 0,
		.product_info_offset = 0,
		.multirecord_offset = 0,
		.pad = 0,
		.checksum = 0xff
};

*/


void fru_read_to_buffer(char *buff, int offset, int length) {

		int i;
		int j = offset;
		//char *write_buffer = buff;
		//char *read_buffer = default_fru+offset;
		for (i = 0; i<length; i++, j++ ) {
			if (j < sizeof(default_fru)) {
				buff[i] = default_fru[j];
			} else {
				buff[i] = 0xFF;
			}

		}


}
