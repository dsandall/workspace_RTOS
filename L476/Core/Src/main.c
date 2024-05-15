
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>
#include <string.h>

#include "timer.h"
#include "usart.h"



/* Private function prototypes -----------------------------------------------*/
void USART_Print(const char* message);
void USART_init(void);
void SystemClock_Config(void);

/**
  * @brief  The application entry point.
  * @retval int
  */

char* inputMessage = "Input (ms): ";


int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  USART_init();

  // Enable MCO, select MSI (4 MHz source)
  RCC->CFGR = ((RCC->CFGR & ~(RCC_CFGR_MCOSEL)) | (RCC_CFGR_MCOSEL_0));

  // Enable GPIOA clock
  RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;

  // Set PA5 as output
  GPIOA->MODER &= ~(0x3 << 5*2);
  GPIOA->MODER |= (0x1 << 5*2);

  TimerSetup();


  USART_Print("\x1B[2J"); // clear screen
  USART_Print("\x1B[0m"); // clear attributes
  USART_Print("\x1B[H");  // move cursor to top left
  USART_Print(inputMessage);
  //  USART_Print("\x1B[3B"); // move cursor down 3 lines
  //  USART_Print("\x1B[5C"); // move cursor right 5 spaces
  //  USART_Print("All good students read the");
  //  USART_Print("\x1B[1B");  // move cursor down 1 lines
  //  USART_Print("\x1B[21D"); // move cursor left 21 lines
  //  USART_Print("\x1B[5m");  // turn on blinking mode
  //  USART_Print("Reference Manual");
  //  USART_Print("\x1B[H");  // move cursor to top left
  //  USART_Print("\x1B[0m"); // clear attributes


}

uint32_t userNumberMS = STARTING_ARR;
static char buffer[11];
char userNumberArr[32];
int userNumPos = 0;
void USART2_IRQHandler(void){
	char r;

	if (USART2->ISR & USART_ISR_RXNE){
		r = USART2->RDR;			// copy received char

//		sprintf(buffer, "0x%X\n", r);
//		USART_Print(buffer);

		if (r == 0xD){
			userNumberArr[userNumPos++] = '\0'; //terminate array
			userNumPos = 0;
			userNumberMS = convertArrayToInt(userNumberArr);
			USART_Print("\x1B[2J"); // clear screen
			USART_Print("\x1B[0m"); // clear attributes
			USART_Print("\x1B[H");  // move cursor to top left
			if (userNumberMS == 0){ //error condition
				USART_Print("Error: Please enter only 0-9, number must be at least 1ms");
				USART_Print("\x1B[H");  // move cursor to top left
				USART_Print("\x1B[1B"); // move cursor down 3 lines
			} else {
				TIM2->ARR = userNumberMS;
				TIM2->CNT = 0;
			}

			sprintf(buffer, "0x%X\n", (unsigned int)userNumberMS);
			USART_Print(buffer);

			USART_Print(inputMessage);

		} else if (r == 0x7F){
			USART_Print("\x1B[1D"); // move cursor left 1 lines
			USART_Print(" ");
			USART_Print("\x1B[1D"); // move cursor left 1 lines

			userNumberArr[--userNumPos] = 0;

		} else {
			USART2->TDR = r; //echo received char
			userNumberArr[userNumPos++] = r;

		}

		USART2->ISR &= ~(USART_ISR_RXNE);	// clear the flag
	}
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
