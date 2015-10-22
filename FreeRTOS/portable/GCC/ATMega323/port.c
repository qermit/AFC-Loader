/* This file has been prepared for Doxygen automatic documentation generation.*/
/*
 * Copyright (C) 2012 Yuriy Kulikov
 *      Universitaet Erlangen-Nuernberg
 *      LS Informationstechnik (Kommunikationselektronik)
 *      Support email: Yuriy.Kulikov.87@googlemail.com
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Compiler definitions include file. */
#include "compiler.h"
#include "FreeRTOSConfig.h"
/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
//#include <tc.h>
#include "pmic.h"
#include "avr/io.h"

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the AVR XMEGA port.
 *----------------------------------------------------------*/

/* Start tasks with interrupts enables. */
#define portFLAGS_INT_ENABLED                    ( ( portSTACK_TYPE ) 0x80 )

/*-----------------------------------------------------------*/

/* We require the address of the pxCurrentTCB variable, but don't want to know
 any details of its type. */
typedef void tskTCB;
extern volatile tskTCB * volatile pxCurrentTCB;

/*-----------------------------------------------------------*/

/* 
 * Macro to save all the general purpose registers, the save the stack pointer
 * into the TCB.  
 * 
 * The first thing we do is save the flags then disable interrupts.  This is to 
 * guard our stack against having a context switch interrupt after we have already 
 * pushed the registers onto the stack - causing the 32 registers to be on the 
 * stack twice. 
 * 
 * r1 is set to zero as the compiler expects it to be thus, however some
 * of the math routines make use of R1. 
 * 
 * The interrupts will have been disabled during the call to portSAVE_CONTEXT()
 * so we need not worry about reading/writing to the stack pointer. 
 */

#define portSAVE_CONTEXT()                                   \
    asm volatile (  "push    r0                     \n\t"    \
                    "in      r0, __SREG__           \n\t"    \
                    "cli                            \n\t"    \
                    "push    r0                     \n\t"    \
                    "push    r1                     \n\t"    \
                    "clr    r1                      \n\t"    \
                    "push    r2                     \n\t"    \
                    "push    r3                     \n\t"    \
                    "push    r4                     \n\t"    \
                    "push    r5                     \n\t"    \
                    "push    r6                     \n\t"    \
                    "push    r7                     \n\t"    \
                    "push    r8                     \n\t"    \
                    "push    r9                     \n\t"    \
                    "push    r10                    \n\t"    \
                    "push    r11                    \n\t"    \
                    "push    r12                    \n\t"    \
                    "push    r13                    \n\t"    \
                    "push    r14                    \n\t"    \
                    "push    r15                    \n\t"    \
                    "push    r16                    \n\t"    \
                    "push    r17                    \n\t"    \
                    "push    r18                    \n\t"    \
                    "push    r19                    \n\t"    \
                    "push    r20                    \n\t"    \
                    "push    r21                    \n\t"    \
                    "push    r22                    \n\t"    \
                    "push    r23                    \n\t"    \
                    "push    r24                    \n\t"    \
                    "push    r25                    \n\t"    \
                    "push    r26                    \n\t"    \
                    "push    r27                    \n\t"    \
                    "push    r28                    \n\t"    \
                    "push    r29                    \n\t"    \
                    "push    r30                    \n\t"    \
                    "push    r31                    \n\t"    \
                    "lds    r26, pxCurrentTCB       \n\t"    \
                    "lds    r27, pxCurrentTCB + 1   \n\t"    \
                    "in        r0, 0x3d             \n\t"    \
                    "st        x+, r0               \n\t"    \
                    "in        r0, 0x3e             \n\t"    \
                    "st        x+, r0               \n\t"    \
                );

/* 
 * Opposite to portSAVE_CONTEXT().  Interrupts will have been disabled during
 * the context save so we can write to the stack pointer. 
 */
#define portRESTORE_CONTEXT()                                \
    asm volatile (  "lds    r26, pxCurrentTCB        \n\t"    \
                    "lds    r27, pxCurrentTCB + 1    \n\t"    \
                    "ld     r28, x+                  \n\t"    \
                    "out    __SP_L__, r28            \n\t"    \
                    "ld     r29, x+                  \n\t"    \
                    "out    __SP_H__, r29            \n\t"    \
                    "pop    r31                      \n\t"    \
                    "pop    r30                      \n\t"    \
                    "pop    r29                      \n\t"    \
                    "pop    r28                      \n\t"    \
                    "pop    r27                      \n\t"    \
                    "pop    r26                      \n\t"    \
                    "pop    r25                      \n\t"    \
                    "pop    r24                      \n\t"    \
                    "pop    r23                      \n\t"    \
                    "pop    r22                      \n\t"    \
                    "pop    r21                      \n\t"    \
                    "pop    r20                      \n\t"    \
                    "pop    r19                      \n\t"    \
                    "pop    r18                      \n\t"    \
                    "pop    r17                      \n\t"    \
                    "pop    r16                      \n\t"    \
                    "pop    r15                      \n\t"    \
                    "pop    r14                      \n\t"    \
                    "pop    r13                      \n\t"    \
                    "pop    r12                      \n\t"    \
                    "pop    r11                      \n\t"    \
                    "pop    r10                      \n\t"    \
                    "pop    r9                       \n\t"    \
                    "pop    r8                       \n\t"    \
                    "pop    r7                       \n\t"    \
                    "pop    r6                       \n\t"    \
                    "pop    r5                       \n\t"    \
                    "pop    r4                       \n\t"    \
                    "pop    r3                       \n\t"    \
                    "pop    r2                       \n\t"    \
                    "pop    r1                       \n\t"    \
                    "pop    r0                       \n\t"    \
                    "out    __SREG__, r0             \n\t"    \
                    "pop    r0                       \n\t"    \
                );

/*-----------------------------------------------------------*/

/*
 * Perform hardware setup to enable ticks from timer 1, compare match A.
 */
static void prvSetupTimerInterrupt(void);
static void prvPortStartFirstTask( void ) __attribute__ (( naked ));
/*-----------------------------------------------------------*/

/* 
 * See header file for description. 
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
	
    /*The addresses are 16 or 24 bit depending on the xmega memory, so use 32 bit variable but put only a
     * part to stack.*/
    uint16_t usAddress;
    /* Place a few bytes of known values on the bottom of the stack.
     This is just useful for debugging. */
    *pxTopOfStack = 0x11;
    pxTopOfStack--;
    *pxTopOfStack = 0x22;
    pxTopOfStack--;
    *pxTopOfStack = 0x33;
    pxTopOfStack--;
    /* Simulate how the stack would look after a call to vPortYield() generated by
     the compiler. */
    /*lint -e950 -e611 -e923 Lint doesn't like this much - but nothing I can do about it. */
    /* The start of the task code will be popped off the stack last, so place
     it on first. */
    /*    Original code
     * For 16-bit program counter (128K program memory or less)
     usAddress = ( unsigned short ) pxCode;
     *pxTopOfStack = ( portSTACK_TYPE ) ( usAddress & ( unsigned short ) 0x00ff );
     pxTopOfStack--;
     usAddress >>= 8;
     *pxTopOfStack = ( portSTACK_TYPE ) ( usAddress & ( unsigned short ) 0x00ff );
     pxTopOfStack--;*/
    /* end of original code block */

    /* The way it should be done for xmega with probably  more than 128K program memory.
     * Warning is OK here - type incompatibility does not matter - usAddress is only
     * used as temporary storage */
    usAddress = (uint16_t)pxCode;

    *pxTopOfStack = (StackType_t ) (usAddress & (uint16_t) 0x00ff);
    pxTopOfStack--;
    usAddress >>= 8;

    *pxTopOfStack = (StackType_t ) (usAddress & (uint16_t) 0x00ff);
    pxTopOfStack--;


#if defined(__AVR_3_BYTE_PC__) && __AVR_3_BYTE_PC__
    *pxTopOfStack = (StackType_t ) 0;
    pxTopOfStack--;
#endif

    /* Next simulate the stack as if after a call to portSAVE_CONTEXT().
     portSAVE_CONTEXT places the flags on the stack immediately after r0
     to ensure the interrupts get disabled as soon as possible, and so ensuring
     the stack use is minimal should a context switch interrupt occur. */
    *pxTopOfStack = (StackType_t ) 0x00; /* R0 */
    pxTopOfStack--;
    *pxTopOfStack = portFLAGS_INT_ENABLED;
    pxTopOfStack--;

    /* Now the remaining registers.   The compiler expects R1 to be 0. */
    *pxTopOfStack = (StackType_t ) 0x00; /* R1 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x02; /* R2 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x03; /* R3 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x04; /* R4 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x05; /* R5 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x06; /* R6 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x07; /* R7 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x08; /* R8 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x09; /* R9 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x10; /* R10 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x11; /* R11 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x12; /* R12 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x13; /* R13 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x14; /* R14 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x15; /* R15 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x16; /* R16 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x17; /* R17 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x18; /* R18 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x19; /* R19 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x20; /* R20 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x21; /* R21 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x22; /* R22 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x23; /* R23 */
    pxTopOfStack--;

    /* Place the parameter on the stack in the expected location. */
    usAddress = (unsigned short) pvParameters;
    *pxTopOfStack = (StackType_t ) (usAddress & (unsigned short) 0x00ff);
    pxTopOfStack--;

    usAddress >>= 8;
    *pxTopOfStack = (StackType_t ) (usAddress & (unsigned short) 0x00ff);
    pxTopOfStack--;

    *pxTopOfStack = (StackType_t ) 0x26; /* R26 X */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x27; /* R27 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x28; /* R28 Y */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x29; /* R29 */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x30; /* R30 Z */
    pxTopOfStack--;
    *pxTopOfStack = (StackType_t ) 0x31; /* R31 */
    pxTopOfStack--;

    return pxTopOfStack;
}
/*-----------------------------------------------------------*/
BaseType_t xPortStartScheduler(void) {
	PORTQ_OUTSET = 1<<2 | 1<<3;
	PORTQ_DIRSET =  1<<2 | 1<<3;
	PORTQ_INT0MASK = 1<<2;
	PORTQ_INT1MASK = 1<<3;
	PORTQ_PIN2CTRL = 0b00110011;
	PORTQ_PIN3CTRL = 0b00110011;
	
	PORTQ_INTCTRL = 0b00000101;
	pmic_enable_level(PMIC_LVL_LOW);
	pmic_set_scheduling(PMIC_SCH_ROUND_ROBIN);
	
	
		
    /* Setup the hardware to generate the tick. */
    prvSetupTimerInterrupt();

	
    /* Restore the context of the first task that is going to run. */
    portRESTORE_CONTEXT();
	
	portENABLE_INTERRUPTS();

	
    /* Simulate a function call end as generated by the compiler.  We will now
     jump to the start of the task the context of which we have just restored. */
    asm volatile ( "ret" );

    /* Should not get here. */
    return pdTRUE;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler(void) {
    /* It is unlikely that the AVR port will get stopped.  If required simply
     disable the tick interrupt here. */
}
/*-----------------------------------------------------------*/

/*
 * Manual context switch.  The first thing we do is save the registers so we
 * can use a naked attribute.
 */
//void vPortYield(void) __attribute__ ( ( naked ) );
void vPortYield(void) {
	/*
	asm volatile ("PUSH R28		\n\t"
"PUSH R29		\n\t"
"IN R28,0x3D		\n\t"
"IN R29,0x3E		\n\t"
//	portNVIC_INT_CTRL_REG_SET = portNVIC_PENDSVSET_BIT_SET;
"LDI R24,0xD2		Load immediate 
"LDI R25,0x07		Load immediate 
"LDI R18,0x73		Load immediate 
"MOVW R30,R24		Copy register pair 
"STD Z+0,R18		Store indirect with displacement 
}
"POP R29		Pop register from stack 
"POP R28		Pop register from stack 
"RET 		Subroutine return */
	//portNVIC_INT_CTRL_REG_SET = portNVIC_PENDSVSET_BIT;
	portNVIC_INT_CTRL_REG_SET = portNVIC_PENDSVSET_BIT_SET;
  //  asm volatile ( "ret" );	
}
/*-----------------------------------------------------------*/


/*-----------------------------------------------------------*/

void vPortYieldFromTick( void ) __attribute__ ( ( naked ) );

/*
 * Setup timer 1 compare match A to generate a tick interrupt.
 */
static void prvSetupTimerInterrupt(void) {
    //Use TCC0 as a tick counter. If this is to be changed, change ISR as well
    TC0_t * tickTimer = &TCC0;
    //select the clock source and pre-scale by 64
	tc_enable(tickTimer);
	
	tc_write_clock_source(tickTimer, TC_CLKSEL_DIV64_gc);
    
    //set period of counter
    tickTimer->PER = configCPU_CLOCK_HZ / configTICK_RATE_HZ / 64 - 1;

    //enable interrupt and set low level
	
	//tc_write_
    //TC0_SetOverflowIntLevel(tickTimer, TC_OVFINTLVL_LO_gc);
    //enable low level interrupts
	tc_set_overflow_interrupt_level(tickTimer, TC_INT_LVL_LO);
	//tc_set_overflow_interrupt_callback(tickTimer, vPortYieldFromTick);
	pmic_enable_level(PMIC_LVL_LOW);
    //PMIC_EnableLowLevel();
}
/*-----------------------------------------------------------*/

volatile uint8_t v1 = 0;
volatile uint8_t v2 = 0;
/* emulate PendSV */
ISR(PORTQ_INT0_vect, ISR_NAKED) {
	// save context first
	portSAVE_CONTEXT();
	
	portNVIC_INT_CTRL_REG_CLR = portNVIC_PENDSVSET_BIT_CLR;
	PORTQ_INTFLAGS = 1<<0;
	
    vTaskSwitchContext();
	
    portRESTORE_CONTEXT();

	asm volatile ("reti");
}

/* emulate SVC */
ISR(PORTQ_INT1_vect) {
	//portNVIC_INT_CTRL_REG_CLR = 1<<3;
	PORTQ_INTFLAGS = 1<<1;
	asm volatile("nop");
	//v2++;
	//asm volatile ("reti");
}


#if configUSE_PREEMPTION == 1

/*
 * Tick ISR for preemptive scheduler.  We can use a naked attribute as
 * the context is saved at the start of vPortYieldFromTick().  The tick
 * count is incremented after the context is saved.
 */


// void vPortYieldFromTick( void )
// {
// 		  portSAVE_CONTEXT();
// 		//  
// 		  xTaskIncrementTick();
// 		  vTaskSwitchContext();
// 		  portRESTORE_CONTEXT();
// 	
// 		  asm volatile ( "reti" );
// }


 ISR (TCC0_OVF_vect) {
     /*
      * Context switch function used by the tick.  This must be identical to
      * vPortYield() from the call to vTaskSwitchContext() onwards.  The only
      * difference from vPortYield() is the tick count is incremented as the
      * call comes from the tick ISR.
      */
     //portSAVE_CONTEXT();
	// Board_LED_Toggle(2);
     xTaskIncrementTick();
	 portNVIC_INT_CTRL_REG_SET = portNVIC_PENDSVSET_BIT_SET;
	 
     //vTaskSwitchContext();
     //portRESTORE_CONTEXT();
	 
   
}

#else

/*
 * Tick ISR for the cooperative scheduler.  All this does is increment the
 * tick count.  We don't need to switch context, this can only be done by
 * manual calls to taskYIELD();
 */
ISR (TCC0_OVF_vect, ISR_NAKED)
{
    xTaskIncrementTick();
}



#endif

