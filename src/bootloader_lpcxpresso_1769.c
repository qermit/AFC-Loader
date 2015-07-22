/*
 * bootloader_lpcxpresso_1769.c
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


#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#error __BYTE_ORDER__
#error __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#endif


#include <cr_section_macros.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "ipmi/ipmi.h"
#include "ipmi/ipmb.h"
#include "ipmi/sdr.h"
#include "ipmi/ipmi_handlers.h"

#define SPEED_100KHZ         100000

#define I2CMODE_POOLING 1
#define I2CMODE_INTERRUPT 0

static int mode_poll;   /* Poll/Interrupt mode flag */

//static int pending_response;

/* Set I2C mode to polling/interrupt */
static void i2c_set_mode(I2C_ID_T id, int polling)
{
	if(!polling) {
		mode_poll &= ~(1 << id);
		Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandler);
		NVIC_EnableIRQ(id == I2C0 ? I2C0_IRQn : I2C1_IRQn);
	} else {
		mode_poll |= 1 << id;
		NVIC_DisableIRQ(id == I2C0 ? I2C0_IRQn : I2C1_IRQn);
		Chip_I2C_SetMasterEventHandler(id, Chip_I2C_EventHandlerPolling);
	}
}

static void i2c_app_init(I2C_ID_T id, int speed, int pooling)
{
	Board_I2C_Init(id);

	/* Initialize I2C */
	Chip_I2C_Init(id);
	Chip_I2C_SetClockRate(id, speed);

	/* Set default mode to interrupt */
	i2c_set_mode(id, pooling);
}

/* State machine handler for I2C0 and I2C1 */
static void i2c_state_handling(I2C_ID_T id)
{
	if (Chip_I2C_IsMasterActive(id)) {
		Chip_I2C_MasterStateHandler(id);
	} else {
		Chip_I2C_SlaveStateHandler(id);
	}
}

/**
 * @brief	I2C0 Interrupt handler
 * @return	None
 */
void I2C0_IRQHandler(void)
{
	i2c_state_handling(I2C0);
}

int main(void) {

#if defined (__USE_LPCOPEN)
#if !defined(NO_BOARD_LIB)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
#endif
#endif


  //  Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_CPU, 1);
  //  Chip_IOCON_PinMux(LPC_IOCON, 1, 27, IOCON_MODE_INACT, IOCON_FUNC1);
  //  Chip_Clock_EnableCLKOUT();

    // TODO: insert code here
    //DEBUGOUT("TEST\n");
	//i2c_app_init(I2C0, SPEED_100KHZ, I2CMODE_INTERRUPT);
    i2c_app_init(I2C0, SPEED_100KHZ, I2CMODE_POOLING);
    //i2c_probe_slaves(I2C0);
    IPMB_init(I2C0);
    DEBUGOUT("\r\nStart MMC\r\n");






    while(1) {
    	if (Chip_I2C_IsStateChanged(I2C0)){
    		i2c_state_handling(I2C0);
    	}
    	IPMI_check_req();
    	asm("nop");
    }

    return 0 ;
}
