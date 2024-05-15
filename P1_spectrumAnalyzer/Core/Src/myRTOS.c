/*
 * myRTOS.c
 *
 *  Created on: May 1, 2024
 *      Author: thebu
 */


#include "main.h"
#include "cmsis_os.h"


volatile unsigned long ulHighFrequencyTimerTicks;

void RTOS_ISR_setPriority(uint32_t IRQn){
	HAL_NVIC_SetPriorityGrouping(0);
	uint32_t lowPriority = NVIC_EncodePriority(0, 10, 0);
	//DMA1_Channel3_IRQn
	NVIC_SetPriority(IRQn, lowPriority);
}

/* Configure Timer to interrupt 100 kHz (100 times every Tick) */
void RTOS_Stats_Timer_Init(void)
{
	RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN);	// turn on TIM2
	TIM2->DIER |= (TIM_DIER_UIE);			// enable interrupts
	TIM2->SR  &= ~(TIM_SR_UIF);				// clear interrupt flag
	TIM2->ARR = 400 - 1;
	TIM2->CR1 |= TIM_CR1_CEN;			    // start timer

	// enable interrupts
	NVIC->ISER[0] = (1 << (TIM2_IRQn & 0x1F));
}

volatile unsigned long ulHighFrequencyTimerTicks;		// run time stats

/* Timer 2 is used to collect runtime stats for FreeRTOS tasks*/
void TIM2_IRQHandler(void)
{
	TIM2->SR &= ~(TIM_SR_UIF);
	ulHighFrequencyTimerTicks++;
}

/* Built in functions for using FreeRTOS runtime stats need to be defined*/
void configureTimerForRunTimeStats(void)
{
    ulHighFrequencyTimerTicks = 0;
    RTOS_Stats_Timer_Init();
}

unsigned long getRunTimeCounterValue(void){
	return ulHighFrequencyTimerTicks;
}




