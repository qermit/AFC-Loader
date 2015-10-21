/*
 * chip.h
 *
 * Created: 2015-10-09 09:10:16
 *  Author: pmiedzik
 */ 


#ifndef CHIP_H_
#define CHIP_H_


#include <asf.h>


#define LPC_SSP0                  ((LPC_SSP_T              *) 0)
#define LPC_SSP1                  ((LPC_SSP_T              *) 0)
//#define LPC_GPIO                  ((LPC_GPIO_T             *) 0)
#define LPC_GPIO                  ((void             *) 0)

#include "chip_types.h"

#include "chip_i2c.h"
#include "chip_ssp.h"
#include "chip_uart.h"


#endif /* CHIP_H_ */