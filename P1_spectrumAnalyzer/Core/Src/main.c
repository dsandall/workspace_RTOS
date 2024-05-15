#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "stm32l4xx.h"


#include "cmsis_os.h"
#include "myRTOS.h"
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core

#include "general.h"
#include "usart.h"
#include "timer.h"

#include "DMA.h"
#include "arm_math.h"
#define FFT_LEN  2048
#define FFT_EXTRA 20
#define HZ_PER_FFT_INDEX 5


void SystemClock_Config(void);

void mallocWrap();
uint16_t *destMem;
float32_t *source1, *dest1, *destMag;
void DMA1_Channel3_IRQHandler();
void doFFT(float32_t* source, float32_t* dest, float32_t* mag, uint32_t len);

//int32_t maxFreq = 0;
//float32_t maxMag = 0.0;




/* USART DRAWING STUFF ------------------------------------------------------*/
#define Y_MAX 56
#define X_MAX 180
#define EDGE_WIDTH 10
#define BIN_COUNT 64
// 85 bin count also works well
#define INDEX_PER_BIN (FFT_LEN/2/BIN_COUNT)
#define SCANNING 1

void printGraph(float32_t *FFT, int32_t len);
void drawBar(int xpos, uint16_t barHeight);
void drawBarScanning(int xpos, uint16_t barHeight, int index);



/* FREERTOS Tasks, Semaphores, and Variables  ------------------------------------*/
void Task_FFT(void *argument);
void Task_USART(void *argument);

TaskHandle_t Task_FFTHandler, task2Handler;
SemaphoreHandle_t FLAG_DMAdone, FLAG_FFTdone;
TickType_t some_ticks = 100/portTICK_PERIOD_MS;



////// Main Function //////
/* Main - just initializations --------------------------------------------------*/
int main(void)
{

	HAL_Init();
	SystemClock_Config(); //overridden by next function

	MSI_Overclock(); //32 MHz
	PortC_Init(); //used for debugging

	USART_init();
	USART_cmd("[40m"); //set color to black for the clear
	USART_wipe(); //wipe the screen of any previously written junk

	mallocWrap(); //malloc arrays in advance

	RTOS_ISR_setPriority(DMA1_Channel3_IRQn); //set priority of interrupt
	DMA1C3_16bADC2M_init(destMem); //set up DMA
	ADC_init_wDMA(); //set up ADC with DMA
	TimerSetup(); //set up Timer for ADC sample frequency


	/* Create the tasks */
	BaseType_t retVal = xTaskCreate(Task_FFT, "Task_FFT", configMINIMAL_STACK_SIZE*3,
			NULL, tskIDLE_PRIORITY + 4, &Task_FFTHandler);
	if (retVal != pdPASS) { while(1);}	// check if task creation failed

	retVal = xTaskCreate(Task_USART, "task2", configMINIMAL_STACK_SIZE*3,
			NULL, tskIDLE_PRIORITY + 3, &task2Handler);
	if (retVal != pdPASS) { while(1);}	// check if task creation failed


	// Create Semaphores for task2 and task3
	FLAG_DMAdone = xSemaphoreCreateBinary();
	if (FLAG_DMAdone == NULL) { while(1); }

	FLAG_FFTdone = xSemaphoreCreateBinary();
	if (FLAG_FFTdone == NULL) { while(1); }


	/* Start scheduler */
	vTaskStartScheduler();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	while (1);
}


////// Task Definitions //////
/* Task_FFT --------------------------------------------------
 * This task starts the ADC when it is first called, and calculates the FFT upon
 *    recieving a semaphore from the DMA_IRQHandler. It also handles conversion from
 *    uint12 to floating point. Upon completion, it gives a semaphore to the Task_USART.
 *    */

void Task_FFT(void *argument)
{

	//start ADC conversions, this only runs once to kick off the process
	ADC3->CR |= ADC_CR_ADSTART;

	for(;;)
	{

		if (xSemaphoreTake(FLAG_DMAdone, some_ticks) == pdPASS) {
			// DMA Semaphore taken, do the FFT
			for (int i = 0; i <FFT_LEN; i++){
				source1[i] = (float32_t)((int16_t)destMem[i]-(0xFFF>>1))/(0xFFF/2);
				//source1[i] = arm_sin_f32(2 * PI * 512 * i / SIN_HZ);
			}

			doFFT(source1, dest1, destMag, FFT_LEN);

			//FIND MAX FREQUENCY
//			maxMag = 0.0;
//			maxFreq = 0;
//			for (int i=1; i<FFT_LEN; i++){ //starts at 1 to ignore DC
//				if(destMag[i] > maxMag){
//					maxMag = destMag[i];
//					maxFreq = i;
//				}
//			}
//
//			printf("Task FFT: is %ld Hz with a shmagnitude of %f \n", maxFreq*HZ_PER_FFT_INDEX, maxMag);
			xSemaphoreGive(FLAG_FFTdone);
		}

	}
}



/* Task_USART --------------------------------------------------
 * This task starts when it recieves the semaphore from Task_FFT. It then calls
 *    printGraph, and upon completion starts the DMA and ADC peripherals again.
 *    */
void Task_USART(void *argument)
{
	for(;;)
	{
		if (xSemaphoreTake(FLAG_FFTdone, some_ticks) == pdPASS) {
			// semaphore taken

			if (SCANNING){

			} else {
				USART_cmd("[40m"); //set color to black for the clear
				USART_wipe();
			}


			printGraph(destMag, 1024);


			if (!SCANNING) {
				vTaskDelay(250 / portTICK_PERIOD_MS);
			}

			//DO IT AGAIN
			DMA1_Channel3->CNDTR = FFT_LEN;
			//set number of transfers before resetting the address(es)

			ADC3->CR |= ADC_CR_ADSTART;
		}
	}

}





////// Misc Functions //////

/* mallocWrap --------------------------------------------------------------
 * This simply mallocs the desired arrays during initialization.
 * The only reason it's a function is to make the main function a little cleaner.
 * */
void mallocWrap(){
	//MALLOC STUFF IN ADVANCE
	destMem = (uint16_t*)malloc(FFT_LEN * sizeof(uint16_t));
	source1 = (float32_t*)malloc((FFT_LEN+FFT_EXTRA) * sizeof(float32_t));
	dest1 = (float32_t*)malloc((FFT_LEN*2+FFT_EXTRA) * sizeof(float32_t));
	destMag = (float32_t*)malloc((FFT_LEN+FFT_EXTRA) * sizeof(float32_t));
	while (destMem == NULL); //loop if malloc failed
	while (source1 == NULL); //loop if malloc failed
	while (dest1 == NULL); //loop if malloc failed
	while (destMag == NULL); //loop if malloc failed
}




/* DMA_IRQHandler --------------------------------------------------------------
 * This is the DMA IRQ handler. It is called upon completion of FFT_LEN samples.
 * It stops the ADC from sampling further,
 *    then passes the semaphore to Task_FFT, and yields to Task_FFT.
 * */
void DMA1_Channel3_IRQHandler(){

	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	if(DMA1->ISR & DMA_ISR_TEIF3){
		//Transfer Error
		while(1);
	}

	if(DMA1->ISR & DMA_ISR_TCIF3){
		//Transfer Complete
		ADC3->CR |= ADC_CR_ADSTP;//stop conversions
		xSemaphoreGiveFromISR(FLAG_DMAdone, &xHigherPriorityTaskWoken);
	}

	DMA1->IFCR |= DMA_IFCR_CGIF3; //clear glbl interrupt flag
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    //yield to whatever task was unblocked by the DMA-set semaphore
}




/* doFFT --------------------------------------------------------------
 * This function takes in a couple pointers to memory arrays,
 *    as well as the length of the FFT.
 * It initializes necessary structures, performs the FFT,
 *    and converts the complex output to magnitude only.
 * The result is read from the *mag pointer.
 * */
void doFFT(float32_t* source, float32_t* dest, float32_t* mag, uint32_t len){

	//initialize structure for FFT
	arm_rfft_fast_instance_f32 f32RFFT;


	arm_status status = arm_rfft_fast_init_f32(&f32RFFT, len);
	if (status != ARM_MATH_SUCCESS){while(1);}

	int IFFTFlag = 0; //1 for inverse FFT
	arm_rfft_fast_f32(&f32RFFT, source, dest, IFFTFlag);

	arm_cmplx_mag_f32(dest, mag, len/2);
}




////// DISPLAY FUNCTIONS //////
/* printGraph --------------------------------------------------------------
 * This function takes in the finished FFT array and it's length,
 *    and calculates the bin magnitude from individual frequency magnitudes.
 * It also calls the drawBar or drawBarScanning functions.
 * */
void printGraph(float32_t *FFT, int32_t len){
	USART_cmd("[45m"); //set color to pink
	for (int i = 1; i < len;){ //starts at 1 to ignore DC

		float32_t binMag = 0.0;
		for(int z = 0; z<INDEX_PER_BIN;z++){
			binMag += FFT[i+z];
		}

		int xpos = (i*2/INDEX_PER_BIN)+EDGE_WIDTH;
		uint16_t barHeight = (uint16_t)(binMag/(float32_t)INDEX_PER_BIN);
		if(SCANNING){
			drawBarScanning(xpos, barHeight, i);
		} else {
			drawBar(xpos, barHeight);
		}

		i += INDEX_PER_BIN;
	}
}

/* drawBar --------------------------------------------------------------
 * This function takes in the x position of a magnitude bar and it's magnitude.
 * It then draws the bar onto the USART terminal.
 * */
void drawBar(int xpos, uint16_t barHeight){

	char barchar = ' ';
	if (barHeight){barHeight = 2*barHeight -1;}
	if (barHeight>Y_MAX){barHeight = Y_MAX;}
	int barY = Y_MAX/2 - barHeight/2;

	USART_YX(barY, xpos);
	for (int i = 0; i<barHeight; i++){
		USART_putchar(barchar);
		USART_cmd("[1D");//back
		USART_cmd("[1B");//down
	}

}

/* drawBarScanning --------------------------------------------------------------
 * This function takes in the x position of a magnitude bar, it's magnitude, and
 *    the index of the first frequency in the bin.
 * It then draws the bar onto the USART terminal.
 * This function "scans" along each bar, top to bottom.
 * The previous bar position is saved so that it can shorten or extend the previous bar
 *    rather than entirely drawing from scratch.
 * The intent was to not induce seizures with the flashing terminal upon refresh.
 * A better implementation would make use of a buffer that allows for horizontal drawing,
 *    as well as detection of any changes so that unnecessary writes could be avoided.
 * This would allow for maximum update rate for a given baud rate.
 * */
int prevBarHeight[FFT_LEN];
void drawBarScanning(int xpos, uint16_t barHeight, int index){

	char barchar = ' ';
	if (barHeight){barHeight = 2*barHeight -1;}
	if (barHeight>Y_MAX){barHeight = Y_MAX;}

	int thisBarY = Y_MAX/2 - barHeight/2;
	int thisBarYMax = Y_MAX/2 + barHeight/2;

	//retrieve past binHeight so it can be cleared efficiently
	int prevBarY = Y_MAX/2 - prevBarHeight[index]/2;
	int prevBarYMax = Y_MAX/2 + prevBarHeight[index]/2;

	int maxBarY;
	int maxBarYMax;

	if(prevBarHeight[index] > barHeight){
		maxBarY = prevBarY;
		maxBarYMax = prevBarYMax;
	} else {
		maxBarY = thisBarY;
		maxBarYMax = thisBarYMax;
	}

	prevBarHeight[index] = barHeight; //save current barHeight

	USART_YX(maxBarY, xpos);
	for (int i = maxBarY; i <= maxBarYMax; i++){
		if ((i>thisBarY) & (i<thisBarYMax)){
			USART_cmd("[45m"); //set color to red
			USART_putchar(barchar);
			USART_cmd("[1D");//back
			USART_cmd("[1B");//down
		} else {
			USART_cmd("[40m"); //set color to black for the clear
			USART_putchar(barchar);
			USART_cmd("[1D");//back
			USART_cmd("[1B");//down

		}
	}
}


















/* -------------STM32 Autogenerated Functions----------------*/






















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
