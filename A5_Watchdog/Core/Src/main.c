
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

#include <stdio.h>
#include "general.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

void TaskInit(void *argument);
void TaskCount(void *argument);

void init_EXTI();

int flag = 0;

uint32_t i = 20000000;

int main(void)
{
	HAL_Init();
	SystemClock_Config();
//	/printf("ljkasdljkasdlkj\n");
	//  void PortA_init(void);


	// Enable GPIOA clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	// Set PA5 as output
	GPIOA->MODER &= ~(0x3 << 5*2);
	GPIOA->MODER |= (0x1 << 5*2);
	GPIOA->ODR |= (GPIO_PIN_5);



	// turn on clock to GPIOC
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	// Configure PC13 for user button input
	GPIOC->MODER &= ~(GPIO_MODER_MODE13);
	GPIOC->MODER |= (0b01<<26);
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD13);
	GPIOC->PUPDR |= (0b01<<26);



	GPIOC->MODER &= ~(GPIO_MODER_MODE0);
	GPIOC->MODER |= 0b01<<0;

//	while (1){
//		if (GPIOC->IDR & 1<<13){
//			GPIOA->ODR = (GPIO_PIN_5);
//		} else{
//			GPIOA->ODR = 0;
//		}
//	}



	init_EXTI();

	__enable_irq();

	while(1){
		if (i < 10){
			GPIOA->ODR &= ~(GPIO_PIN_5);
		}else{
			GPIOA->ODR |= (GPIO_PIN_5);
			i--;
		}
	}


//  /* Create the tasks */
//  BaseType_t retVal;	// used for checking task creation
//
//  retVal = xTaskCreate(TaskInit, "task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, &task1Handler);
//  if (retVal != pdPASS) { while(1);}	// check if task creation failed
//  retVal = xTaskCreate(TaskCount, "task2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, &task1Handler);
//  if (retVal != pdPASS) { while(1);}	// check if task creation failed
//
//
//  vTaskStartScheduler();
}

void TaskInit(void *argument){

}

void TaskCount(void *argument){

}

void init_IWDG(){

}

void init_EXTI(){
	RCC->APB2ENR |= 0b1<<0; //enable SYSCFG clock

	//turn on clock
	EXTI->IMR1 |= 0b1<<13; //enable interrupts for line 13 (line 0-15 dedicated to GPIO)
	EXTI->FTSR1 |= 0b1<<13; //falling edge generates interrupt
	SYSCFG->EXTICR[3] &= 0b1111<<4;
	SYSCFG->EXTICR[3] |= 0b0010<<4; //set port c for line 13

	//off by one error?
	NVIC->ISER[EXTI15_10_IRQn / 32] = (1 << (EXTI15_10_IRQn & 0x1F)); //enable interrupts in the NVIC

		EXTI->PR1 |= 1<<13; //clear interrupt
}

void EXTI15_10_IRQHandler(){
	GPIOC->BSRR |= 1;
	if (EXTI->PR1 & 1<<13){
		EXTI->PR1 |= 1<<13; //clear interrupt

		i = 20000;
	} else {
		while(1){

			GPIOA->ODR |= (GPIO_PIN_5);
			HAL_Delay(1);
			GPIOA->ODR &= ~(GPIO_PIN_5);
			HAL_Delay(1);

		}
	}
	GPIOC->BRR |= 1;
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
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 40;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM16 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
//void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
//{
//  /* USER CODE BEGIN Callback 0 */
//
//  /* USER CODE END Callback 0 */
//  if (htim->Instance == TIM16) {
//    HAL_IncTick();
//  }
//  /* USER CODE BEGIN Callback 1 */
//
//  /* USER CODE END Callback 1 */
//}

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