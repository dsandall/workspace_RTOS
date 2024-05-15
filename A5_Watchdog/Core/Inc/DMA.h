/*
 * DMA.h
 *
 *  Created on: Apr 13, 2024
 *      Author: thebu
 */

#ifndef SRC_DMA_H_
#define SRC_DMA_H_

void DMA_registers();
void DMA1C1_32bM2M_init(uint16_t* sourceAddress, uint16_t* destAddress, uint16_t transfers);
void DMA1C3_16bADC2M_init(uint16_t* destAddress);
void ADC_init_wDMA();


#endif /* SRC_DMA_H_ */
