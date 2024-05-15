/*
 * general.c
 *
 *  Created on: Apr 22, 2024
 *      Author: thebu
 */

#include "main.h"


void PortC_Init(void)
{
	// turn on clock to GPIOC
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

	// Configure PC0-3 for GPIO output
	GPIOC->MODER   &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 |
					    GPIO_MODER_MODE2 | GPIO_MODER_MODE3);
	GPIOC->MODER   |=  ((1 << GPIO_MODER_MODE0_Pos) |
					    (1 << GPIO_MODER_MODE1_Pos) |
					    (1 << GPIO_MODER_MODE2_Pos) |
					    (1 << GPIO_MODER_MODE3_Pos));

	GPIOC->OTYPER  &= ~(GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 |
					    GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);
	GPIOC->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED0 | GPIO_OSPEEDR_OSPEED1 |
					    GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED3);
	GPIOC->PUPDR   &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 |
					    GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);
	GPIOC->BSRR    =   (GPIO_BSRR_BR0 | GPIO_BSRR_BR1 |
					    GPIO_BSRR_BR2 | GPIO_BSRR_BR3);

	// Configure PC13 for user button input
	GPIOC->MODER &= ~(GPIO_MODER_MODE13);
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD13);
}

void PortA_init(void){
	// Configure PA5 for LED output
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	GPIOA->MODER   &= ~(GPIO_MODER_MODE5);
	GPIOA->MODER   |=  (1 << GPIO_MODER_MODE5_Pos);
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED5);
	GPIOA->OTYPER  &= ~(GPIO_OTYPER_OT5);
	GPIOA->ODR     &= ~(GPIO_PIN_5);
}

void MSI_Overclock(void){

	//
	//Change MSI speed
	//

//	uint8_t MSIRANGE_400khz =0b0010;
//	uint8_t MSIRANGE_4Mhz =0b0110;
	uint8_t MSIRANGE_32Mhz = 0b1010;
	  if (RCC->CR & 0b1) { //if MSI clock is in RDY state
		  RCC->CR &= ~(0b1111 << 4); //clear MSI freq select register
		  RCC->CR |= (MSIRANGE_32Mhz << 4); //set it to desired Frequency
		  RCC->CR |= (0b1 << 3); //enable MSI frequency selection
	  }


	//Output MSI on MCO (Pin A8)
	  RCC->CFGR |= 0b100<<28; //prescale MCO by 16
	  RCC->CFGR |= 0b0001<<24; //SYSCLOCK on MCO
	  //RCC->CFGR & RCC_CFGR_SWS //system clock status register (read which clock is sysclock)
	  RCC->CFGR &= ~(0b11); //system clock switch (select which clock is sysclock)

	  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	  GPIOA->MODER &= ~(0b11<<(8*2)); //set port A8 to AF mode
	  GPIOA->MODER |= 0b10<<(8*2); //set port A8 to AF mode
	  GPIOA->AFR[0] &= ~(0b1111 << 0); //Clear AF select reg
	  GPIOA->AFR[0] |= (0b0000 << 0); //Port A8 to AFR 0

}

//for SW debug printing
int _write(int file, char *ptr, int len)
{
  (void)file;
  int DataIdx;

  for (DataIdx = 0; DataIdx < len; DataIdx++)
  {
    ITM_SendChar(*ptr++);
  }
  return len;
}
