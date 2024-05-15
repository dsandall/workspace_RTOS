#include <stdlib.h>
#include "main.h"
#include "DMA.h"

uint8_t DMA_Flag = 0;

void DMA1_Channel3_IRQHandler(){

	if(DMA1->ISR & DMA_ISR_TEIF3){
		//Transfer Error
		DMA_Flag = 2;
		//DMA1->IFCR |= DMA_IFCR_CTEIF3;
	}

	if(DMA1->ISR & DMA_ISR_TCIF3){
		//Transfer Complete
		//DMA1->IFCR |= DMA_IFCR_CTCIF3;
		GPIOC->ODR ^= (1<<1); //Toggle PC1
		DMA_Flag = 1;
	}

	if(DMA1->ISR & DMA_ISR_HTIF3){
		//Transfer Half Complete
		//DMA1->IFCR |= DMA_IFCR_CHTIF3;
		GPIOC->ODR ^= (1<<2); //Toggle PC2
		DMA_Flag = 3;
	}

	DMA1->IFCR |= DMA_IFCR_CGIF3; //clear glbl interrupt flag



}

void PortC_Init(void)
{
	// turn on clock to GPIOC
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;

	// Configure PC5 for GPIO output to control LED
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
}


int main(void)
{

	HAL_Init();
	PortC_Init();

	uint16_t arraySize = 1000;
	uint16_t *destMem = (uint16_t*)malloc(arraySize * sizeof(uint16_t));
	while (destMem == NULL); //loop if malloc failed

	*destMem = 0;
	for (int i = 0; i <arraySize; i++){
		destMem[i] = 0;
	}



	DMA1C3_16bADC2M_init(destMem);
	ADC_init_wDMA();


	while (1)
	{
	}
  /* USER CODE END 3 */
}




/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
