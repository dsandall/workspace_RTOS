/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : FreeRTOS program to demonstrate semaphores. The program
  *                   creates 3 tasks. The first task will read the user button
  *                   on the Nucelo board. If the button is pressed, it will
  *                   give a semaphore for task 2 (task2Sema). If the button is
  *                   not pressed, it gives a semaphore to task 3 (task3Sema).
  *                   Task 2 and Task 3 each wait for their respective
  *                   semaphores. If they get a semaphore, they toggle a pin in
  *                   port C (Task 2 = PC0, Task 3 = PC2). If they time out
  *                   waiting for the semaphore (after 100 ticks) a different
  *                   pin in port C is toggled (Task 2 = PC1, Task 3 = PC3)
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "event_groups.h"               // ARM.FreeRTOS::RTOS:Event Groups
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PortC_Init(void);
void RTOS_Stats_Timer_Init(void);

/* Task function prototypes  ------------------------------------------------*/
void Task1(void *argument);
void Task2(void *argument);
void Task3(void *argument);

/* Global Variables    -----------------------------------------------------*/
volatile unsigned long ulHighFrequencyTimerTicks;		// run time stats
TaskHandle_t task1Handler, task2Handler, task3Handler;
SemaphoreHandle_t task2Sema, task3Sema;
TickType_t shortTimeOut = 50;		// portMAX_DELAY

/* main --------------------------------------------------------------------*/
int main(void)
{
  BaseType_t retVal;	// used for checking task creation

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  // Initialize GPIO
  PortC_Init();

  // Create the tasks
  retVal = xTaskCreate(Task1, "task1", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, &task1Handler);
  if (retVal != pdPASS) { while(1);}	// check if task creation failed

  retVal = xTaskCreate(Task2, "task2", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 5, &task2Handler);
  if (retVal != pdPASS) { while(1);}	// check if task creation failed

  retVal = xTaskCreate(Task3, "task3", configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 4, &task3Handler);
  if (retVal != pdPASS) { while(1);}	// check if task creation failed

  // Create Semaphores for task2 and task3
  task2Sema = xSemaphoreCreateBinary();
  if (task2Sema == NULL) { while(1); }

  task3Sema = xSemaphoreCreateBinary();
  if (task3Sema == NULL) { while(1); }

  // Start scheduler
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  while (1)
  {
  }
}

/* Define Tasks --------------------------------------------------------------*/
// Task will toggle the on board LED to show it is running. If the user button
//   is pressed, it will give task2 semaphore. If the button isn't pressed it
//   will give the task3 semaphore
void Task1(void *argument)
{
	for(;;)
	{
		// Blink LED on PA5
		GPIOA->ODR ^= GPIO_ODR_OD5;

		if (GPIOC->IDR & GPIO_PIN_13) {		// check button press
			//Button not pressed
			xSemaphoreGive(task2Sema);
		} else {
			//Button pressed
			xSemaphoreGive(task3Sema);
		}
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
}

// Task will toggle PC0 when it receives task2 semaphore
//  if it times out waiting for the semaphore it will toggle PC1
void Task2(void *argument)
{
	for(;;)
	{								 // portMAX_DELAY
		if (xSemaphoreTake(task2Sema, shortTimeOut) == pdPASS) {
			GPIOC->ODR ^= GPIO_ODR_OD0;			// toggle PC0
		}
		else {
			GPIOC->ODR ^= GPIO_ODR_OD1;			// toggle PC1
		}
	}
}

// Task will toggle PC2 when it receives task3 semaphore
//  if it times out waiting for the semaphore it will toggle PC3
void Task3(void *argument)
{
	for(;;)
	{								 // portMAX_DELAY
		if (xSemaphoreTake(task3Sema, shortTimeOut) == pdPASS) {
			GPIOC->ODR ^= GPIO_ODR_OD2;			// toggle PC2
		}
		else {
			GPIOC->ODR ^= GPIO_ODR_OD3;			// toggle PC3
		}	}
}
/*
 * Configure PC0-PC3 for GPIO Output
 * push-pull, low speed, no pull-up/pull-down resistors
 * Initialize all to 0s
 */
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
	GPIOC->BSRR    =   (GPIO_BSRR_BR0 | GPIO_BSRR_BR1 |
					    GPIO_BSRR_BR2 | GPIO_BSRR_BR3);

	// Configure PC13 for user button input
	GPIOC->MODER &= ~(GPIO_MODER_MODE13);
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD13);

	// Configure PA5 for LED output
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	GPIOA->MODER   &= ~(GPIO_MODER_MODE5);
	GPIOA->MODER   |=  (1 << GPIO_MODER_MODE5_Pos);
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED5);
	GPIOA->OTYPER  &= ~(GPIO_OTYPER_OT5);
	GPIOA->ODR     &= ~(GPIO_PIN_5);
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

unsigned long getRunTimeCounterValue(void)
{
	return ulHighFrequencyTimerTicks;
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
  RCC_OscInitStruct.PLL.PLLN = 20;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
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
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM16) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
