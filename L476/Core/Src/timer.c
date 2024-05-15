#include "main.h"
#include "timer.h"


/*
counter upticks at 4mhz, and input provided as ms (one UEV period)
counter UEV period (seconds) = (ARR+1) * (PSC+1) / 4 MHz
thus, if input is ARR, PSC = s/ms * 4MHz = (1/1,000)*4,000,000
*/
const int prescale = 4000;


void TimerSetup(void){
//  TIMER ESSENTIALS
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN; //Enable Clock input to TIM2
	TIM2->PSC = prescale; // Set prescaler value
	TIM2->ARR = STARTING_ARR-1;   // Set auto-reload value
	TIM2->CR1 |= TIM_CR1_CEN; // Enable TIM2 in config register 1
	TIM2->CR1 &= ~(TIM_CR1_DIR);  // Clear DIR bit to set upcount mode (not super essential, cuz it defaults here)

	//// 	Enable TIM2 interrupt in NVIC (THE interrupter for STM32)
	NVIC->ISER[TIM2_IRQn / 32] = (1 << (TIM2_IRQn % 32));

//	TIM2->DIER |= 6; // enable interrupt generation for CC channels 1 and 2
//	TIM2->CCR1 = (STARTING_ARR/4)-1; //interrupt at 25%
//	TIM2->CCR2 = 0; // interrupt at 0% (this could also just be the overrun flag, but eh)

	TIM2->DIER |= TIM_DIER_UIE; //enable update interrupts
}

void TIM2_IRQHandler(void) {
	TIM2->SR &= ~TIM_SR_UIF; //clear flag
	GPIOA->ODR ^= (1<<5); //toggle bit 5

//  //For CCR interrupts Mode
//	if(TIM2->SR & TIM_SR_CC1IF){
//		TIM2->SR &= ~TIM_SR_CC1IF; //clear flag
//		GPIOA->ODR &= ~(1 << 5);//set low
//	}
//
//	if (TIM2->SR & TIM_SR_CC2IF){
//		TIM2->SR &= ~TIM_SR_CC2IF; //clear flag
//		GPIOA->ODR |= (1 << 5);//set high
//	}
}
