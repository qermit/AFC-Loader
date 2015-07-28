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

#ifdef USE_FREERTOS
#warning "MMC Verion"
#include "FreeRTOS.h"
#include "task.h"
#else
#warning "BOOTLOADER Verion"
#endif

#define SPEED_100KHZ         100000
#define SPEED_100KHZ         90000

#define I2CMODE_POOLING 1
#define I2CMODE_INTERRUPT 0

static int mode_poll;   /* Poll/Interrupt mode flag */


#define DELAY_PERIOD 1000

#ifdef FREERTOS_CONFIG_H

void LEDTask( void *pvParmeters )
{
TickType_t xLastWakeTime;

    xLastWakeTime = xTaskGetTickCount();

    for( ;; )
    {
        // Wait for the next cycle.
        vTaskDelayUntil( &xLastWakeTime, DELAY_PERIOD );
        Board_LED_Toggle(0);
       // Board_LED_Toggle(2);
    }
}
#endif



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
		Board_LED_Toggle(1);

		Chip_I2C_MasterStateHandler(id);
	} else {
		//Board_LED_Toggle(2);
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
//	portYIELD();
}

//static void delay(  ) {
//uint16_t i = 0;
//uint8_t z = 0;
//
//	for(i=0; i<20000; i++)
//	{
//		for(z=0; z<100; z++)
//		{
//
//		}
//	}
//
//}

int main(void) {
	__disable_irq();
#if defined (__USE_LPCOPEN)
#if !defined(NO_BOARD_LIB)
    // Read clock settings and update SystemCoreClock variable
    SystemCoreClockUpdate();
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();
    // Set the LED to the state of "On"
    Board_LED_Set(0, true);
    Board_LED_Set(1, false);
    Board_LED_Set(2, false);

#endif
#endif


  //  Chip_Clock_SetCLKOUTSource(SYSCTL_CLKOUTSRC_CPU, 1);
  //  Chip_IOCON_PinMux(LPC_IOCON, 1, 27, IOCON_MODE_INACT, IOCON_FUNC1);
  //  Chip_Clock_EnableCLKOUT();

    // TODO: insert code here
    //DEBUGOUT("TEST\n");
    IPMI_init();

#ifdef FREERTOS_CONFIG_H
    __enable_irq();
	i2c_app_init(I2C0, SPEED_100KHZ, I2CMODE_INTERRUPT);
#else
    i2c_app_init(I2C0, SPEED_100KHZ, I2CMODE_POOLING);
#endif
    //i2c_probe_slaves(I2C0);
    IPMB_init(I2C0);
    DEBUGOUT("\r\nStart MMC\r\n");

//    struct ipmi_msg *pmsg = IPMI_alloc();
//    struct ipmi_ipmb_addr *dst_addr = &pmsg->daddr;
//    struct ipmi_ipmb_addr *src_addr = &pmsg->saddr;
//    src_addr->lun = 0;
//    src_addr->slave_addr = 0x76;
//    dst_addr->lun = 0;
//    dst_addr->slave_addr = 0x20;
//    pmsg->msg.lun = 0;
//    pmsg->msg.netfn = NETFN_SE;
//    pmsg->msg.cmd = IPMI_PLATFORM_EVENT_CMD;
//    int data_len = 0;
//    pmsg->msg_data[data_len++] = 0x04;
//    pmsg->msg_data[data_len++] = 0xf2;
//    pmsg->msg_data[data_len++] = 0;
//    pmsg->msg_data[data_len++] = 0x6f;
//    pmsg->msg_data[data_len++] = 0x00; // hot swap state
//    pmsg->msg.data_len = data_len;
////
////    Board_LED_Set(2,1);
////    Board_LED_Toggle(1);
//    IPMB_send(pmsg);
//    test_var = 2;
//	Board_LED_Set(2,0);
//    delay();
//    Board_LED_Set(2,1);
//    test_var = 3;
//    Board_LED_Toggle(1);
//    IPMB_send(pmsg);
//    Board_LED_Set(2,0);
//    test_var = 4;
//
//    IPMI_free(pmsg);


#ifdef FREERTOS_CONFIG_H
    TaskHandle_t xLedHandle = NULL;
    TaskHandle_t xIPMIHandle = NULL;

    xTaskCreate(LEDTask, "LED",configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY, &xLedHandle );

    xTaskCreate(vTaskIPMI, "IPMI",configMINIMAL_STACK_SIZE*5, NULL,  tskIDLE_PRIORITY, &xIPMIHandle );
    if( xIPMIHandle != NULL )
         {
    	//     vTaskDelete( xIPMIHandle );
         }

    vTaskStartScheduler();
#else
    while(1) {
    	if (Chip_I2C_IsStateChanged(I2C0)){
    		i2c_state_handling(I2C0);
    	}
    	IPMI_check_req();
    	IPMI_send_proc();
    	asm("nop");
    }
#endif
    for( ;; );
    return 0 ;
}

#ifdef FREERTOS_CONFIG_H
void vApplicationTickHook( void )
{
	static unsigned long ulTicksSinceLastDisplay = 0;

	/* Called from every tick interrupt as described in the comments at the top
	of this file.

	Have enough ticks passed to make it	time to perform our health status
	check again? */
//	ulTicksSinceLastDisplay++;
//	if( ulTicksSinceLastDisplay >= mainCHECK_DELAY )
//	{
//		/* Reset the counter so these checks run again in mainCHECK_DELAY
//		ticks time. */
//		ulTicksSinceLastDisplay = 0;
//
//		/* Has an error been found in any task? */
//		if( xAreGenericQueueTasksStillRunning() != pdTRUE )
//		{
//			pcStatusMessage = "An error has been detected in the Generic Queue test/demo.";
//		}
//		else if( xAreQueuePeekTasksStillRunning() != pdTRUE )
//		{
//			pcStatusMessage = "An error has been detected in the Peek Queue test/demo.";
//		}
//		else if( xAreBlockingQueuesStillRunning() != pdTRUE )
//		{
//			pcStatusMessage = "An error has been detected in the Block Queue test/demo.";
//		}
//		else if( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
//		{
//			pcStatusMessage = "An error has been detected in the Block Time test/demo.";
//		}
//	    else if( xAreSemaphoreTasksStillRunning() != pdTRUE )
//	    {
//	        pcStatusMessage = "An error has been detected in the Semaphore test/demo.";
//	    }
//	    else if( xArePollingQueuesStillRunning() != pdTRUE )
//	    {
//	        pcStatusMessage = "An error has been detected in the Poll Queue test/demo.";
//	    }
//	    else if( xAreIntegerMathsTaskStillRunning() != pdTRUE )
//	    {
//	        pcStatusMessage = "An error has been detected in the Int Math test/demo.";
//	    }
//	    else if( xAreRecursiveMutexTasksStillRunning() != pdTRUE )
//	    {
//	    	pcStatusMessage = "An error has been detected in the Mutex test/demo.";
//	    }
//	}
}
/*-----------------------------------------------------------*/
void vApplicationStackOverflowHook( TaskHandle_t pxTask, char *pcTaskName )
{
	/* This function will get called if a task overflows its stack. */

	( void ) pxTask;
	( void ) pcTaskName;

	for( ;; );
}
/*-----------------------------------------------------------*/
void vConfigureTimerForRunTimeStats( void )
{
const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00, TCR_COUNT_ENABLE = 0x01;

	/* This function configures a timer that is used as the time base when
	collecting run time statistical information - basically the percentage
	of CPU time that each task is utilising.  It is called automatically when
	the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
	to 1). */

	/* Power up and feed the timer. */
	LPC_SYSCTL->PCONP |= 0x02UL;
	LPC_SYSCTL->PCLKSEL[0] = (LPC_SYSCTL->PCLKSEL[0] & (~(0x3<<2))) | (0x01 << 2);

	/* Reset Timer 0 */
	LPC_TIMER0->TCR = TCR_COUNT_RESET;

	/* Just count up. */
	LPC_TIMER0->CTCR = CTCR_CTM_TIMER;

	/* Prescale to a frequency that is good enough to get a decent resolution,
	but not too fast so as to overflow all the time. */
	LPC_TIMER0->PR =  ( configCPU_CLOCK_HZ / 10000UL ) - 1UL;

	/* Start the counter. */
	LPC_TIMER0->TCR = TCR_COUNT_ENABLE;
}
/*-----------------------------------------------------------*/


TickType_t getTickDifference(TickType_t current_time, TickType_t start_time) {
	TickType_t result = 0;
	if (current_time < start_time) {
		result = start_time - current_time;
		result = portMAX_DELAY - result;
	} else {
		result = current_time - start_time;
	}
	return result;
}

#endif

