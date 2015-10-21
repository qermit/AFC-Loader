/*
 * chip_ssp.h
 *
 * Created: 2015-10-09 09:22:25
 *  Author: pmiedzik
 */ 


#ifndef CHIP_SSP_H_
#define CHIP_SSP_H_



typedef struct {			/*!< SSPn Structure         */
	__IO uint32_t CR0;		/*!< Control Register 0. Selects the serial clock rate, bus type, and data size. */
	__IO uint32_t CR1;		/*!< Control Register 1. Selects master/slave and other modes. */
	__IO uint32_t DR;		/*!< Data Register. Writes fill the transmit FIFO, and reads empty the receive FIFO. */
	__I  uint32_t SR;		/*!< Status Register        */
	__IO uint32_t CPSR;		/*!< Clock Prescale Register */
	__IO uint32_t IMSC;		/*!< Interrupt Mask Set and Clear Register */
	__I  uint32_t RIS;		/*!< Raw Interrupt Status Register */
	__I  uint32_t MIS;		/*!< Masked Interrupt Status Register */
	__O  uint32_t ICR;		/*!< SSPICR Interrupt Clear Register */
	__IO uint32_t DMACR;	/*!< SSPn DMA control register */
} LPC_SSP_T;

#endif /* CHIP_SSP_H_ */