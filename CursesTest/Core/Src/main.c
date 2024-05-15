
#include "main.h"


#include "usart.h"

#include <stdio.h>
#include <stdarg.h>

#define Y_MAX 56
#define X_MAX 180
#define EDGE_WIDTH 6
#define F_BINS 64
#define MAG_MAX 16

void SystemClock_Config(void);

void drawBar(int xpos, uint16_t barHeight){

	char barchar = ' ';
	if (barHeight){barHeight = 2*barHeight -1;}
	int barY = Y_MAX/2 - barHeight/2;
	USART_cmd("[45m"); //set color to red

	USART_YX(barY, xpos);
	for (int i = 0; i<barHeight; i++){
		USART_putchar(barchar);
		USART_cmd("[1D");

		USART_cmd("[1B");

	}



}


void MSI_Overclock(void){

	//
	//Change MSI speed
	//
	uint8_t MSIRANGE_400khz =0b0010;
	uint8_t MSIRANGE_4Mhz =0b0110;
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





int main(void)
{
	HAL_Init();
	SystemClock_Config();
	MSI_Overclock();


	USART_init();
	USART_cmd("[40m"); //set text background to black
	USART_clear();




	int i, z;
	i = 0;
	z= 4;

	while (1)
	{
		for (int i =0; i<16; i++){
			USART_print("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
			USART_cmd("[H"); //set text background to black
			for (int z=0; z<i; z++){
				USART_cmd("[1B"); //set text background to black
			}
		}
		HAL_Delay(5000);
		USART_clear();

		//maybe instead of clearing, which makes this flash effect, just use a scrolling buffer that overwrites?







//
//		//USART_YX(z++,i++);
//		drawBar(EDGE_WIDTH+(i++), z++);
//
//		if (z>MAG_MAX){z=0;};
//
//		if (i > X_MAX-EDGE_WIDTH){
//			i = 0; z = 0;
//			HAL_Delay(1000);
//			USART_cmd("[40m"); //set text background to black
//			USART_clear();
//		}
	}
  /* USER CODE END 3 */
}


















































/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
