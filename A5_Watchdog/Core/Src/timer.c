#include "main.h"
#include "timer.h"




/*
 * Setting timer frequency:
 * Update Event rate (hz) = input clock freq/((PSC+1)*(ARR+1))
 *
 *
 * UpCOUNT event (hz) = (input clock freq)/PSC+1, but this is usually not the one we want
 */



/*
 * TLDR: if you set prescale to 4000, then your ARR is 1/2 the period (length of on) for a UEV
 *
 * counter upticks at 4mhz, and input provided as ms (one UEV period)
 * counter UEV period (seconds) = (ARR+1) * (PSC+1) / 4 MHz
 * thus, if input is ARR, PSC = s/ms * 4MHz = (1/1,000)*4,000,000
 *
*/

#define MY_ARR 1953 //max of 2^32
#define MY_PRESCALE 7; //0+1 for 4MHz, 7+1 for 32MHz
//#define STARTING_ARR 100 //max of 2^32
//const int prescale = 4000;


void TimerSetup(void){
//  TIMER ESSENTIALS
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN; //Enable Clock input to TIM2
	TIM2->PSC = MY_PRESCALE; // Set prescaler value
	TIM2->ARR = MY_ARR;   // Set auto-reload value
	TIM2->CR1 &= ~(TIM_CR1_DIR);  // Clear DIR bit to set upcount mode (not super essential, cuz it defaults here)

	NVIC->ISER[TIM2_IRQn / 32] = (1 << (TIM2_IRQn & 0x1F)); // -> (TIM2_IRQn % 32)

//	TIM2->DIER |= 6; // enable interrupt generation for CC channels 1 and 2
//	TIM2->CCR1 = (STARTING_ARR/4)-1; //interrupt at 25%
//	TIM2->CCR2 = 0; // interrupt at 0% (this could also just be the overrun flag, but eh)

//	TIM2->DIER |= TIM_DIER_UIE; //enable update interrupts

	TIM2->CCR2 = 0;

	TIM2->CCMR1 &= ((0<<24) |(0b11<<12) | (00<<8)); //TODO: this was previously &= ~(), which would result in badness... maybe this was the fix?
	TIM2->CCMR1 |= ((0<<24) |(0b11<<12) | (00<<8));
		//OC2M = 0001 //channel active when CNT = CCR2
		//CC2S = 00 //channel is output mode

	//TIM2->DIER |= TIM_DIER_CC2IE | TIM_DIER_TIE;



	//TIM2->CCER |= TIM_CCER_CC2P; //polarity? not sure exactly what this does for output mode... investigate if broken
	TIM2->CCER |= TIM_CCER_CC2E;//enable CC2 (must be after configuration

	//what is OC2XE (CC2 clear enable, preload enable, fast enable)

	TIM2->CR1 |= TIM_CR1_CEN; // Enable TIM2 in config register 1

}

void AnotherTimerSetup(void){
//  TIMER ESSENTIALS
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN; //Enable Clock input to TIM2
	TIM5->PSC = 0; // Set prescaler value
	TIM5->ARR = 0xFFFFFF;   // Set auto-reload value
	TIM5->CR1 &= ~(TIM_CR1_DIR);  // Clear DIR bit to set upcount mode (not super essential, cuz it defaults here)

	//NVIC->ISER[TIM2_IRQn / 32] = (1 << (TIM2_IRQn & 0x1F)); // -> (TIM2_IRQn % 32)

//	TIM2->DIER |= 6; // enable interrupt generation for CC channels 1 and 2
//	TIM2->CCR1 = (STARTING_ARR/4)-1; //interrupt at 25%
//	TIM2->CCR2 = 0; // interrupt at 0% (this could also just be the overrun flag, but eh)

//	TIM2->DIER |= TIM_DIER_UIE; //enable update interrupts

	//TIM5->CCR2 = 0;

	//TIM5->CCMR1 &= ((0<<24) |(0b11<<12) | (00<<8)); //TODO: this was previously &= ~(), which would result in badness... maybe this was the fix?
	//TIM5->CCMR1 |= ((0<<24) |(0b11<<12) | (00<<8));
		//OC2M = 0001 //channel active when CNT = CCR2
		//CC2S = 00 //channel is output mode

	//TIM2->DIER |= TIM_DIER_CC2IE | TIM_DIER_TIE;



	//TIM2->CCER |= TIM_CCER_CC2P; //polarity? not sure exactly what this does for output mode... investigate if broken
	//TIM5->CCER |= TIM_CCER_CC2E;//enable CC2 (must be after configuration

	//what is OC2XE (CC2 clear enable, preload enable, fast enable)

//	TIM5->CR1 |= TIM_CR1_CEN;

}

void test_init_pinA5onboardLED(void){
	  // Enable GPIOA clock
	  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

	  // Set PA5 as output
	  GPIOA->MODER &= ~(0x3 << 5*2);
	  GPIOA->MODER |= (0x1 << 5*2);
}

//void TIM2_IRQHandler(void) {
//////For UEV/UIF interrupts Mode
////	TIM2->SR &= ~TIM_SR_UIF; //clear flag
////	GPIOA->ODR ^= (1<<5); //toggle bit 5
//
//////For CCR1 interrupts Mode
////	if(TIM2->SR & TIM_SR_CC1IF){
////		TIM2->SR &= ~TIM_SR_CC1IF; //clear flag
////		GPIOA->ODR &= ~(1 << 5);//set low
////	}
//
//
//////For CCR2 interrupt mode
////	test_init_pinA5onboardLED();
//	if (TIM2->SR & TIM_SR_CC2IF){
//		TIM2->SR &= ~TIM_SR_CC2IF; //clear flag
////		GPIOA->ODR ^= (1 << 5); //toggle onboard LED
////		ADC3->CR |= ADC_CR_ADSTART;
//
//	}
//
//	if(TIM2->SR & TIM_SR_TIF){
//		TIM2->SR &= ~TIM_SR_TIF;
//	}
//
//
//}
