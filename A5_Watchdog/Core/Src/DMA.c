/*
 * DMA.c
 *
 *  Created on: Apr 13, 2024
 *      Author: thebu
 */

#include "main.h"

void DMA_registers(){
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN; //Enable Clock input to DMA1

	//these are shared across channels of DMA1
	DMA1->ISR |= 0x0;	//shows current interrupt flags
	DMA1->IFCR |= 0x0; //clears interrupt flags
	DMA1_CSELR->CSELR |= 0x0; //Shouldn't matter for M2M transfers


	//these are individual to each of the 8 channels on DMA1
	DMA1_Channel1->CCR |= 0x0; //
	DMA1_Channel1->CNDTR |= 0x0;
	DMA1_Channel1->CPAR |= 0x0;
	DMA1_Channel1->CMAR |= 0x0;



	NVIC->ISER[DMA1_Channel1_IRQn / 32] = (1 << (DMA1_Channel1_IRQn % 32));
}


void DMA1C1_32bM2M_init(uint16_t* sourceAddress, uint16_t* destAddress, uint16_t transfers){
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN; //Enable Clock input to DMA1


	DMA1_Channel1->CNDTR = transfers; //set to 500 32bit transfers

	DMA1_Channel1->CPAR = (uint32_t)sourceAddress; //Peripheral Address (DIR=0: Source address)
	DMA1_Channel1->CMAR = (uint32_t)destAddress; //Memory Address(DIR=0: Destination address)

	DMA1_Channel1->CCR = 0x0; //clear entire register
	DMA1_Channel1->CCR |= DMA_CCR_MEM2MEM | DMA_CCR_TCIE | DMA_CCR_TEIE | (0b10<<12); //Priority high
	DMA1_Channel1->CCR |= ((0b10<<8) | (0b10<<10)); //sets PSIZE and MSIZE to 32 bit chunks
	DMA1_Channel1->CCR |= DMA_CCR_PINC | DMA_CCR_MINC; //enable incrementation for source and destination (effected by DIR)
	//enabling/disabling DMA_CCR_DIR merely flips direction of the transfer (DIR=0: P2M)


	NVIC->ISER[DMA1_Channel1_IRQn / 32] = (1 << (DMA1_Channel1_IRQn % 32));
	//TODO: Ensure that ISR is in place, handling TCIF1 and TEIF1

	DMA1_Channel1->CCR |= DMA_CCR_EN; //must be enabled after configuration

}

void DMA1C3_16bADC2M_init(uint16_t* destAddress){
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN; //Enable Clock input to DMA1

	DMA1_Channel3->CNDTR = 2048; //number of transfers before resetting the address(es)

	DMA1_Channel3->CPAR = (uint32_t)&(ADC3->DR); //Peripheral Address (DIR=0: Source address)
	DMA1_Channel3->CMAR = (uint32_t)destAddress; //Memory Address(DIR=0: Destination address)

	DMA1_Channel3->CCR = 0x0; //clear entire register
	DMA1_Channel3->CCR |= DMA_CCR_TCIE | DMA_CCR_TEIE | DMA_CCR_CIRC | (0b10<<12); //Priority high
	DMA1_Channel3->CCR |= ((0b01<<8) | (0b01<<10)); //sets PSIZE and MSIZE to 16 bit chunks (S and D for dir=0)
	DMA1_Channel3->CCR &= ~(DMA_CCR_PINC); //disable incrementation for source
	DMA1_Channel3->CCR |= (DMA_CCR_MINC); //enable incrementation for destination

	//enabling/disabling DMA_CCR_DIR merely flips direction of the transfer (DIR=0: P2M)

	DMA1_CSELR->CSELR = (0b0000<<8); //set channel 3 interrupt triggering to ADC3

	NVIC->ISER[DMA1_Channel3_IRQn / 32] = (1 << (DMA1_Channel3_IRQn % 32));
	//TODO: Ensure that ISR is in place, handling TCIF1, THIF1, and TEIF1

	DMA1_Channel3->CCR |= DMA_CCR_EN; //must be enabled after configuration

}


void ADC3_IRQHandler(void) {
////For UEV/UIF interrupts Mode
//	TIM2->SR &= ~TIM_SR_UIF; //clear flag
//	GPIOA->ODR ^= (1<<5); //toggle bit 5

////For CCR1 interrupts Mode
//	if(TIM2->SR & TIM_SR_CC1IF){
//		TIM2->SR &= ~TIM_SR_CC1IF; //clear flag
//		GPIOA->ODR &= ~(1 << 5);//set low
//	}



	  ADC3->ISR |= ADC_ISR_EOS;
	  ADC3->ISR |= ADC_ISR_EOSMP;


	//For CCR2 interrupt mode
	if (ADC3->ISR & ADC_ISR_EOC){
		ADC3->ISR |= ADC_ISR_EOC; //clear flag
		GPIOA->ODR ^= (1 << 5); //toggle onboard LED
	}
	if (ADC3->ISR & ADC_ISR_ADRDY){
		ADC3->ISR |= ADC_ISR_ADRDY; //clear flag
		GPIOA->ODR ^= (1 << 5); //toggle onboard LED
	}
	if (ADC3->ISR & ADC_ISR_OVR){
		ADC3->ISR |= ADC_ISR_OVR; //clear flag
		GPIOA->ODR ^= (1 << 5); //toggle onboard LED
	}


}


void ADC_registers(){
	  RCC->AHB2ENR |= (1<<13);  // enable ADC clock
	  ADC123_COMMON->CCR |= 0x0;
	  ADC3->CFGR |= 0x0;
}

void ADC_init_wDMA(){
	  //Enable ADC and GPIO clock, set up PC0
	  RCC->AHB2ENR |= (1<<13);  // enable ADC clock
	  RCC->AHB2ENR |= (1<<2);  // enable GPIOC clock
	  ADC123_COMMON->CCR |= ADC_CCR_CKMODE; //alternative clock mode (HCLK/4)

		// turn on clock to GPIOC
	  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	  //set up GPIO pin
	  int pinCx = 0; //pinC0
	  GPIOC->MODER |= (0b11 << (pinCx*2)); //set C0 to analog mode
	  GPIOC->ASCR |= GPIO_ASCR_ASC0;

	  //shouldn't matter for ADC3 (if you switch to ADC1/2 this might matter, and could be in the wrong sequence)
	  ADC3->DIFSEL = 0x0; //single ended mode for all channels

	  //initialize ADC
	  ADC3->CR &= ~ADC_CR_DEEPPWD; //disable DeepPowerDown mode (defaults to on)
	  ADC3->CR |= ADC_CR_ADVREGEN; //enable ADC voltage regulator (required)
	  for(int i = 20; i > 0; i--){asm("NOP");} //required delay is 20us, one clock period @4MHz is .25us

	  //calibrate ADC
	  ADC3->CR &= ~(ADC_CR_ADEN | ADC_CR_ADCALDIF);
	  ADC3->CR |= ADC_CR_ADCAL;
	  while ((ADC3->CR & ADC_CR_ADCAL)){}; //wait for calibration




	  //enable ADC
	  ADC3->ISR |= ADC_ISR_ADRDY; //clear RDY
	  ADC3->CR |= ADC_CR_ADEN; //enable ADC after regulator startup
	  while(!(ADC3->ISR & ADC_ISR_ADRDY)){}; //wait for ready
	  ADC3->ISR |= ADC_ISR_ADRDY; //clear RDY



	  //Interrupts
//	  NVIC->ISER[ADC3_IRQn / 32] = (1 << (ADC3_IRQn & 0x1F)); //enable interrupts
//	  ADC3->IER |= 1; //interrupt on ADRDY
//	  ADC3->IER |= 1<<2; //interrupt on conversion complete
//	  ADC3->IER |= 1<<4; //interrupt on overrun


	  //ADC hardware triggering (clears CFGR!)
	  ADC3->CFGR &= (0011<<6); //EXTSEL = 0011 (TIM2CH2)
	  ADC3->CFGR |= (0b0011<<6); //EXTSEL = 0011 (TIM2CH2)
	  ADC3->CFGR &= ~ADC_CFGR_EXTEN; //hardware triggers conversions (on rising edge)
	  	  ADC3->CFGR |= 0b11<<10; //hardware triggers conversions (on rising edge) TODO: not enabling?

	  //set up ADC (DMA, continuous mode, resolution)
	  ADC3->CFGR |= ADC_CFGR_DMAEN; //enable DMA triggering //no longer ADC_CFGR_DMACFG, no longer continuous
	  ADC3->CFGR &= ~ADC_CFGR_RES; //set resolution to 12b





	  //set sample time for channel 1
	  int channel = 1; //channel 1
	  ADC3->SMPR1 = 0b100<<(3*channel); //set sample time (47.5 ADC clock cycles = some frequency)

	  //set conversion sequence
	  ADC3->SQR1 = (0b1<<6) | (0b0<<0); //set first conversion in sequence to channel 1, set conversion sequence length to 1 conversion (just convert channel 1 over and over)










	  //ADC3->CR |= ADC_CR_ADSTP; //stop ADC conversions

	  //myData = ADC3->DR; //read ADC3

}






