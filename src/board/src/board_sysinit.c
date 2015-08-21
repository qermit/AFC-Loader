/*
 * @brief AFCv2 initialization file modified by Piotr Miedzik <P.Miedzik@gsi.de>
 *
 * @note
 * Copyright(C) NXP Semiconductors, 2013
 * All rights reserved.
 *
 * @par
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * LPC products.  This software is supplied "AS IS" without any warranties of
 * any kind, and NXP Semiconductors and its licensor disclaim any and
 * all warranties, express or implied, including all implied warranties of
 * merchantability, fitness for a particular purpose and non-infringement of
 * intellectual property rights.  NXP Semiconductors assumes no responsibility
 * or liability for the use of the software, conveys no license or rights under any
 * patent, copyright, mask work right, or any other intellectual property rights in
 * or to any products. NXP Semiconductors reserves the right to make changes
 * in the software without notification. NXP Semiconductors also makes no
 * representation or warranty that such application will be suitable for the
 * specified use without further testing or modification.
 *
 * @par
 * Permission to use, copy, modify, and distribute this software and its
 * documentation is hereby granted, under NXP Semiconductors' and its
 * licensor's relevant copyrights in the software, without fee, provided that it
 * is used in conjunction with NXP Semiconductors microcontrollers.  This
 * copyright, permission, and disclaimer notice must appear in all copies of
 * this code.
 */

#include "board.h"

/* The System initialization code is called prior to the application and
 initializes the board for run-time operation. Board initialization
 includes clock setup and default pin muxing configuration. */

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/* AFCv2
I2C1 (P0[0], P0[1])
     00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
====================================================
00                           -- -- -- -- -- -- -- --
01   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
02   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
03   30 -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
04   40 41 42 43 44 45 -- -- 48 -- -- -- 4C 4D 4E 4F
05   50 -- -- -- -- -- -- 57 58 -- -- -- -- -- -- --
06   -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
07   -- -- -- -- -- -- -- -- --
*/



/* Pin muxing configuration */
STATIC const PINMUX_GRP_T pinmuxing[] = {

//{ 0, 1, IOCON_MODE_INACT | IOCON_FUNC3 }, /* temp_SDA */
//{ 0, 2, IOCON_MODE_INACT | IOCON_FUNC3 }, /* temp_SCL */

		{ 0, 2, IOCON_MODE_INACT | IOCON_FUNC1 }, /* PROG_TXD */
		{ 0, 3, IOCON_MODE_INACT | IOCON_FUNC1 }, /* PROG_TXD */
		{ 0, 4, IOCON_MODE_INACT | IOCON_FUNC0 }, /* EN_FMC1_P12V */
		{ 0, 5, IOCON_MODE_INACT | IOCON_FUNC0 }, /* EN_FMC2_P12V */

		{ 0, 6, IOCON_MODE_INACT | IOCON_FUNC0 }, /* FCS_B_L */
		{ 0, 7, IOCON_MODE_INACT | IOCON_FUNC0 }, /* FPGA_CCLQ_L */
		{ 0, 8, IOCON_MODE_INACT | IOCON_FUNC0 }, /* FLASH_Q/D1_L */
		{ 0, 9, IOCON_MODE_INACT | IOCON_FUNC0 }, /* FLASH_SI/D0_L */

		//{ 0, 10, IOCON_MODE_INACT | IOCON_FUNC2 }, /* SDA2 */
		//{ 0, 11, IOCON_MODE_INACT | IOCON_FUNC2 }, /* SCL2 */

		//{ 0, 15, IOCON_MODE_INACT | IOCON_FUNC2 }, /* DAC_VADJ_CLK */
		//{ 0, 16, IOCON_MODE_INACT | IOCON_FUNC2 }, /* DAC_VADJ_CSn */

		{ 0, 17, IOCON_MODE_INACT | IOCON_FUNC0 }, /* PROGRAM_B_L */

		//{ 0, 18, IOCON_MODE_INACT | IOCON_FUNC2 }, /* DAC_VADJ_SDI */

		//{ 0, 19, IOCON_MODE_INACT | IOCON_FUNC2 }, /* SDA1 */
		//{ 0, 20, IOCON_MODE_INACT | IOCON_FUNC2 }, /* SCL1 */

		{ 0, 21, IOCON_MODE_INACT | IOCON_FUNC0 }, /* DAC_VADJ_RSTn */

		{ 0, 22, IOCON_MODE_INACT | IOCON_FUNC0 }, /* DONE_L */

		{ 0, 23, IOCON_MODE_INACT | IOCON_FUNC0 }, /* EN_P1V2 */
		{ 0, 24, IOCON_MODE_INACT | IOCON_FUNC0 }, /* EN_P1V8 */
		{ 0, 25, IOCON_MODE_INACT | IOCON_FUNC0 }, /* EN_FMC1_P3V3 */
		{ 0, 26, IOCON_MODE_INACT | IOCON_FUNC0 }, /* EN_FMC2_P3V3 */
		//{ 0, 27, IOCON_MODE_INACT | IOCON_FUNC1 }, /* SDA0 */
		//{ 0, 28, IOCON_MODE_INACT | IOCON_FUNC1 }, /* SCL0 */

		/* DC-DC */


		{ 1, 0, IOCON_MODE_INACT | IOCON_FUNC0 }, /* GA0_R */
		{ 1, 1, IOCON_MODE_INACT | IOCON_FUNC0 }, /* GA1_R */
		{ 1, 4, IOCON_MODE_INACT | IOCON_FUNC0 }, /* GA2_R */
		{ 1, 8, IOCON_MODE_INACT | IOCON_FUNC0 }, /* P1_R */
		{ 1, 9, IOCON_MODE_INACT | IOCON_FUNC0 }, /* P2_R */
		{ 1, 10, IOCON_MODE_INACT | IOCON_FUNC0 }, /* P3_R */
		{ 1, 14, IOCON_MODE_INACT | IOCON_FUNC0 }, /* FMC1_PRSNT_M2C_R */
		{ 1, 15, IOCON_MODE_INACT | IOCON_FUNC0 }, /* FMC2_PRSNT_M2C_R */
		{ 1, 16, IOCON_MODE_INACT | IOCON_FUNC0 }, /* FMC1_PG_M2C_R */
		{ 1, 17, IOCON_MODE_INACT | IOCON_FUNC0 }, /* FMC2_PG_M2C_R */
		{ 1, 18, IOCON_MODE_INACT | IOCON_FUNC0 }, /* FMC1_PG_C2M_R */
		{ 1, 19, IOCON_MODE_INACT | IOCON_FUNC0 }, /* FMC2_PG_C2M_R */
		{ 1, 20, IOCON_MODE_INACT | IOCON_FUNC3 }, /* FPGA_SCK_L */
		{ 1, 21, IOCON_MODE_INACT | IOCON_FUNC3 }, /* FPGA_SSEL_L */
		{ 1, 22, IOCON_MODE_INACT | IOCON_FUNC0 }, /* RESETn */
		{ 1, 23, IOCON_MODE_INACT | IOCON_FUNC3 }, /* FPGA_MISO_L */
		{ 1, 24, IOCON_MODE_INACT | IOCON_FUNC3 }, /* FPGA_MOSI_L */
		{ 1, 25, IOCON_MODE_INACT | IOCON_FUNC0 }, /* P4 */
		{ 1, 26, IOCON_MODE_PULLUP | IOCON_FUNC0 }, /* UPDATEn */
		{ 1, 27, IOCON_MODE_INACT | IOCON_FUNC0 }, /* EN_P3V3 */
		{ 1, 28, IOCON_MODE_INACT | IOCON_FUNC0 }, /* EN_FMC2_PVADJ */
		{ 1, 29, IOCON_MODE_INACT | IOCON_FUNC0 }, /* 1V5_VTT_EN */
		{ 1, 31, IOCON_MODE_INACT | IOCON_FUNC0 }, /* EN_FMC1_PVADJ */

		/* SCANSTA */
		{ 2, 0, IOCON_MODE_INACT | IOCON_FUNC0 }, /* SCANSTA_ADR0 */
		{ 2, 1, IOCON_MODE_INACT | IOCON_FUNC0 }, /* SCANSTA_ADR1 */
		{ 2, 2, IOCON_MODE_INACT | IOCON_FUNC0 }, /* SCANSTA_ADR2 */
		{ 2, 3, IOCON_MODE_INACT | IOCON_FUNC0 }, /* SCANSTA_ADR3 */
		{ 2, 4, IOCON_MODE_INACT | IOCON_FUNC0 }, /* SCANSTA_ADR4 */
		{ 2, 5, IOCON_MODE_INACT | IOCON_FUNC0 }, /* SCANSTA_ADR5 */
		{ 2, 6, IOCON_MODE_INACT | IOCON_FUNC0 }, /* SCANSTA_ADR6 */
		{ 2, 7, IOCON_MODE_INACT | IOCON_FUNC0 }, /* SCANSTA_RSTn */

		{ 2, 8, IOCON_MODE_INACT | IOCON_FUNC0 }, /* ENABLE# */
		{ 2, 9, IOCON_MODE_INACT | IOCON_FUNC0 }, /* FPGA_RESETn */
		{ 2, 10, IOCON_MODE_INACT | IOCON_FUNC0 }, /* BOOT */
		{ 2, 11, IOCON_MODE_INACT | IOCON_FUNC0 }, /* OVERTEMPn */
		{ 2, 12, IOCON_MODE_INACT | IOCON_FUNC0 }, /* SW */
		{ 2, 13, IOCON_MODE_INACT | IOCON_FUNC0 }, /* MOD HANDLE */

		{ 3, 25, IOCON_MODE_INACT | IOCON_FUNC0 }, /* EN_P1V0 */
		{ 3, 26, IOCON_MODE_INACT | IOCON_FUNC0 }, /* PGOOD_P1V0 */

		{ 4, 28, IOCON_MODE_INACT | IOCON_FUNC3 }, /* TXD3 */
		{ 4, 29, IOCON_MODE_INACT | IOCON_FUNC3 }, /* RXD3 */
};

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* Sets up system pin muxing */
void Board_SetupMuxing(void) {
	Chip_IOCON_SetPinMuxing(LPC_IOCON, pinmuxing,
			sizeof(pinmuxing) / sizeof(PINMUX_GRP_T));
}

/* Setup system clocking */
void Board_SetupClocking(void) {
	Chip_SetupXtalClocking();

	/* Setup FLASH access to 4 clocks (100MHz clock) */
	Chip_SYSCTL_SetFLASHAccess(FLASHTIM_100MHZ_CPU);
}

/* Set up and initialize hardware prior to call to main */
void Board_SystemInit(void) {
	Board_SetupMuxing();
	Board_SetupClocking();
}
