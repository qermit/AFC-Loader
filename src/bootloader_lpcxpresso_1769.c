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


//#if defined (__USE_LPCOPEN)
//#if defined(NO_BOARD_LIB)
//#include "chip.h"
//#else
#include "board.h"
//#endif
//#endif

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#error __BYTE_ORDER__
#error __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#endif


/*#include <cr_section_macros.h>*/
#include <string.h>
#include <stdint.h>
#include "board.h"

#include "ipmi/ipmi.h"
#include "ipmi/ipmb.h"
#include "ipmi/sdr.h"
#include "ipmi/ipmi_handlers.h"
#include "ipmi/ipmi_oem.h"
#include "ipmi/payload.h"
#include "afc/board_version.h"

#ifdef USE_FREERTOS
#warning "MMC Verion"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "freertos_private.h"

#else
#warning "BOOTLOADER Verion"
#endif

#define DELAY_PERIOD 1000

#ifdef FREERTOS_CONFIG_H


static const ipmiFuncEntry_t const  __attribute__ ((section (".ipmi_handlers"))) ipmiEntries[] = {
// 		{ NETFN_APP,     IPMI_GET_DEVICE_ID_CMD, ipmi_get_device_id},
// 		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_GET_PROPERTIES, ipmi_picmg_get_PROPERTIES},
// 		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_FRU_CONTROL, ipmi_picmg_cmd_fru_control},
// 		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_SET_FRU_LED_STATE, ipmi_picmg_set_fru_led_state},
// 		{ NETFN_GRPEXT,  IPMI_PICMG_CMD_GET_DEVICE_LOCATOR_RECORD, ipmi_picmg_get_device_locator_record},
		//{ NETFN_GRPEXT,  IPMI_PICMG_CMD_SET_AMC_PORT_STATE, ipmi_picmg_cmd_set_amc_port_state },
		//{ NETFN_GRPEXT,  IPMI_PICMG_CMD_GET_TELCO_ALARM_CAPABILITY, ipmi_picmg_cmd_get_telco_alarm_capability},
		/*{ NETFN_SE,      IPMI_SET_EVENT_RECEIVER_CMD, ipmi_se_set_event_reciever},*/
//		{ NETFN_SE,      IPMI_GET_DEVICE_SDR_INFO_CMD, ipmi_se_get_sdr_info},
// 		{ NETFN_SE,      IPMI_GET_DEVICE_SDR_CMD, ipmi_se_get_sdr},
// 		{ NETFN_SE,		 IPMI_GET_SENSOR_READING_CMD, ipmi_se_get_sensor_reading},
// 		{ NETFN_SE,      IPMI_RESERVE_DEVICE_SDR_REPOSITORY_CMD, ipmi_se_reserve_device_sdr},

// 		{ NETFN_STORAGE, IPMI_GET_FRU_INVENTORY_AREA_INFO_CMD, ipmi_storage_get_fru_info},
// 		{ NETFN_STORAGE, IPMI_READ_FRU_DATA_CMD, ipmi_storage_read_fru_data_cmd},
// 		{ NETFN_CUSTOM_AFC, IPMI_AFC_CMD_I2C_TRANSFER, ipmi_afc_i2c_transfer},
// 		//{ NETFN_CUSTOM_AFC, IPMI_AFC_CMD_GPIO, ipmi_afc_gpio},
// 		{ NETFN_CUSTOM_AFC, IPMI_AFC_CMD_SSP_TRANSFER, ipmi_afc_ssp_transfer},
 		{ NETFN_CUSTOM_AFC, IPMI_AFC_CMD_SSP_TRANSFER_RAW, NULL},
};

void LEDTask( void *pvParmeters )
{
TickType_t xLastWakeTime;
		struct afc_generic_task_param *xParams = (struct afc_generic_task_param *) pvParmeters;

/*		
		while(xSemaphoreTake(xParams->afc_init_semaphore, portMAX_DELAY) == pdFALSE ) {
			asm("nop");
		}*/
		xSemaphoreGive(xParams->afc_init_semaphore);
		
    xLastWakeTime = xTaskGetTickCount();
	const ipmiFuncEntry_t *  p_ptr;
	p_ptr = (ipmiFuncEntry_t *) &_ipmi_handlers;
	
	ipmiFuncEntry_t current_entry;
	int i = 0;
	current_entry.cmd = ipmiEntries[i].cmd;
	
    for( ;; )
    {
				
	
		memcpy_P(&current_entry, p_ptr, sizeof(current_entry));
		// Wait for the next cycle.
        vTaskDelayUntil( &xLastWakeTime, DELAY_PERIOD );
        Board_LED_Toggle(0);
       // Board_LED_Toggle(2);
	   p_ptr++;
	   if (p_ptr >= (ipmiFuncEntry_t *) &_eipmi_handlers){
		   p_ptr = (ipmiFuncEntry_t *) &_ipmi_handlers;
	   }
    }
}
#endif


/* State machine handler for I2C0 and I2C1 */
static void i2c_state_handling(I2C_ID_T id)
{
	//if (Chip_I2C_IsMasterActive(id)) {
		//Board_LED_Toggle(1);

		//Chip_I2C_MasterStateHandler(id);
	//} else {
		//Board_LED_Toggle(2);
		//Chip_I2C_SlaveStateHandler(id);
	//}
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

void I2C1_IRQHandler(void)
{
	i2c_state_handling(I2C1);
//	portYIELD();
}


void I2C2_IRQHandler(void)
{
	i2c_state_handling(I2C2);
//	portYIELD();
}


void reset_FPGA(void)
{
	/*
	Chip_GPIO_SetPinDIR(LPC_GPIO, 0, 6, true);
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, false );
	delay(  );
	Chip_GPIO_SetPinState(LPC_GPIO, 0, 6, true );
	Chip_GPIO_SetPinDIR(LPC_GPIO, 0, 6, false);
	*/
}



SemaphoreHandle_t afc_init_semaphore;

int main(void) {
	cpu_irq_disable();
#if defined (__USE_LPCOPEN)
#if !defined(NO_BOARD_LIB)
    // Read clock settings and update SystemCoreClock variable
    //SystemCoreClockUpdate();
    // Set up and initialize all required blocks and
    // functions related to the board hardware
    Board_Init();

    Board_LED_Set(1, true);
    initializeDCDC();
#endif
#endif

#ifdef FREERTOS_CONFIG_H
	vSemaphoreCreateBinary(afc_init_semaphore);
	if (afc_init_semaphore == NULL)	 {
		asm("nop");
	}
	xSemaphoreTake(afc_init_semaphore, 0);

    cpu_irq_enable();
	i2c_app_init(I2C0, SPEED_100KHZ, I2CMODE_INTERRUPT);


	afc_board_i2c_init();

	Board_SSP_Init(LPC_SSP1);
	//Chip_SSP_Init(LPC_SSP1);
	//Chip_SSP_Enable(LPC_SSP1);
	//Chip_SSP_SetMaster(LPC_SSP1, 1);
	create_ssp1_mutex();
    DEBUGOUT("\r\nAFC/AFCK MMC");

#else
    i2c_app_init(I2C0, SPEED_100KHZ, I2CMODE_POOLING);
    DEBUGOUT("\r\nAFC/AFCK Bootloader ");

#endif

	DEBUGOUT(" Copyright (C) 2015  Piotr Miedzik <P.Miedzik@gsi.de>\r\n");
	DEBUGOUT("This program comes with ABSOLUTELY NO WARRANTY;\r\n");
	DEBUGOUT("This is free software, and you are welcome to redistribute it\r\n");
	DEBUGOUT("under certain conditions;\r\n");


    afc_board_discover();

//	NVIC_ClearPendingIRQ(EINT2_IRQn);
//	NVIC_SetPriority(EINT2_IRQn, 0);
//	LPC_SYSCTL->EXTINT = 1UL << 2;
//	LPC_SYSCTL->EXTMODE = 1UL << 2;
//	LPC_SYSCTL->EXTPOLAR = 0UL << 2;
//	NVIC_EnableIRQ(EINT2_IRQn);

  //  Chip_GPIO_SetPinState(LPC_GPIO, 1, 22, false);
//	Chip_GPIO_SetPinDIR(LPC_GPIO, 1, 22, true);
	//asm("nop");
	//Chip_GPIO_SetPinState(LPC_GPIO, 1, 22, true);


    IPMI_init();
    unsigned char ipmi_slave_addr = IPMB_init(I2C0);
	ipmi_slave_addr = 0x76;
	sdr_init(ipmi_slave_addr);

	{
		struct ipmi_ipmb_addr tmp_src;
		struct ipmi_ipmb_addr tmp_dst;
		tmp_src.lun = 0;
		tmp_src.slave_addr = ipmi_slave_addr;
		tmp_dst.lun = 0;
		tmp_dst.slave_addr = 0x20;

		IPMI_evet_set_address(&tmp_src,&tmp_dst);
	}



#ifdef FREERTOS_CONFIG_H
    TaskHandle_t xLedHandle = NULL;
    TaskHandle_t xIPMIHandle = NULL;
    TaskHandle_t xSensorHandle = NULL;
    TaskHandle_t xPayloadHandle = NULL;
	struct afc_generic_task_param  xLedHandleParams;
	struct afc_generic_task_param  xIPMIHandleParams;
	struct afc_generic_task_param  xSensorHandleParams;
	struct afc_generic_task_param  xPayloadHandleParams;
	xLedHandleParams.afc_init_semaphore = afc_init_semaphore;
	xIPMIHandleParams.afc_init_semaphore = afc_init_semaphore;
	xSensorHandleParams.afc_init_semaphore = afc_init_semaphore;
	xPayloadHandleParams.afc_init_semaphore = afc_init_semaphore;

    do_quiesced_init();

    xTaskCreate(LEDTask, "LED", configMINIMAL_STACK_SIZE, &xLedHandleParams, tskIDLE_PRIORITY, &xLedHandle );
    xTaskCreate(vTaskIPMI, "IPMI", configMINIMAL_STACK_SIZE*5, &xIPMIHandleParams,  tskIDLE_PRIORITY, &xIPMIHandle );
    xTaskCreate(vTaskSensor, "Sensor", configMINIMAL_STACK_SIZE, &xSensorHandleParams,  tskIDLE_PRIORITY, &xSensorHandle );
    xTaskCreate(vTaskPayload, "Payload", configMINIMAL_STACK_SIZE, &xPayloadHandleParams, tskIDLE_PRIORITY, &xPayloadHandle);

	
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
// const unsigned long TCR_COUNT_RESET = 2, CTCR_CTM_TIMER = 0x00, TCR_COUNT_ENABLE = 0x01;
// 
// 	/* This function configures a timer that is used as the time base when
// 	collecting run time statistical information - basically the percentage
// 	of CPU time that each task is utilising.  It is called automatically when
// 	the scheduler is started (assuming configGENERATE_RUN_TIME_STATS is set
// 	to 1). */
// 
// 	/* Power up and feed the timer. */
// 	LPC_SYSCTL->PCONP |= 0x02UL;
// 	LPC_SYSCTL->PCLKSEL[0] = (LPC_SYSCTL->PCLKSEL[0] & (~(0x3<<2))) | (0x01 << 2);
// 
// 	/* Reset Timer 0 */
// 	LPC_TIMER0->TCR = TCR_COUNT_RESET;
// 
// 	/* Just count up. */
// 	LPC_TIMER0->CTCR = CTCR_CTM_TIMER;
// 
// 	/* Prescale to a frequency that is good enough to get a decent resolution,
// 	but not too fast so as to overflow all the time. */
// 	LPC_TIMER0->PR =  ( configCPU_CLOCK_HZ / 10000UL ) - 1UL;
// 
// 	/* Start the counter. */
// 	LPC_TIMER0->TCR = TCR_COUNT_ENABLE;
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

