/*
 * payload.h
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

#ifndef IPMI_PAYLOAD_H_
#define IPMI_PAYLOAD_H_

enum payload_state {
	PAYLOAD_NO_POWER = 0,
	PAYLOAD_SWITCHING_ON = 1,
	PAYLOAD_POWER_GOOD_WAIT = 3,
	PAYLOAD_STATE_FPGA_SETUP = 4,
	PAYLOAD_FPGA_BOOTING = 5,
	PAYLOAD_FPGA_WORKING = 6,
	PAYLOAD_SWITCHING_OFF = 7,
	PAYLOAD_QUIESCED = 8,
	PAYLOAD_STATE_NO_CHANGE = 253,
	PAYLOAD_STATE_UNKNOWN = 254,
	PAYLOAD_POWER_FAIL = 255
};

enum payload_message {
	PAYLOAD_MESSAGE_P12GOOD = 0,
	PAYLOAD_MESSAGE_P12GOODn = 1,
	PAYLOAD_MESSAGE_PGOOD = 2,
	PAYLOAD_MESSAGE_PGOODn = 3,
	PAYLOAD_MESSAGE_COLD_RST = 4,
	PAYLOAD_MESSAGE_WARM_RST = 5,
	PAYLOAD_MESSAGE_REBOOT   = 6,
	PAYLOAD_MESSAGE_QUIESCED = 7,

};

#define FRU_CTLCODE_COLD_RST          (0)       // FRU Control command cold reset code
#define FRU_CTLCODE_WARM_RST          (1)       // FRU Control command warm reset code
#define FRU_CTLCODE_REBOOT            (2)       // FRU Control command reboot code
#define FRU_CTLCODE_QUIESCE           (4)       // FRU Control command quiesce code

// @todo: move to board definition

#define GPIO_EN_P1V2_PORT       0
#define GPIO_EN_P1V2_PIN       23
#define GPIO_EN_P1V8_PORT       0
#define GPIO_EN_P1V8_PIN       24

#define GPIO_EM_FMC1_P12V_PORT  0
#define GPIO_EM_FMC1_P12V_PIN   4
#define GPIO_EN_FMC1_P3V3_PORT  0
#define GPIO_EN_FMC1_P3V3_PIN  25
#define GPIO_EN_FMC1_PVADJ_PORT 1
#define GPIO_EN_FMC1_PVADJ_PIN  31

#define GPIO_EM_FMC2_P12V_PORT  0
#define GPIO_EM_FMC2_P12V_PIN   5
#define GPIO_EN_FMC2_P3V3_PORT  0
#define GPIO_EN_FMC2_P3V3_PIN  26
#define GPIO_EN_FMC2_PVADJ_PORT 1
#define GPIO_EN_FMC2_PVADJ_PIN 28


#define GPIO_EN_P3V3_PORT       1
#define GPIO_EN_P3V3_PIN       27
#define GPIO_EN_1V5_VTT_PORT    1
#define GPIO_EN_1V5_VTT_PIN    29
#define GPIO_EN_P1V0_PORT       3
#define GPIO_EN_P1V0_PIN       25

#define GPIO_PROGRAM_B_PIN     17
#define GPIO_PROGRAM_B_PORT     0
#define GPIO_DONE_B_PIN        22
#define GPIO_DONE_B_PORT        0
#define GPIO_PGOOD_P1V0_PIN    26
#define GPIO_PGOOD_P1V0_PORT    3


void payload_send_message(uint8_t msg);
void vTaskPayload(void *pvParmeters);


#endif /* IPMI_PAYLOAD_H_ */
