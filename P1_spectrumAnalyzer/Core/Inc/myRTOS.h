/*
 * myRTOS.h
 *
 *  Created on: May 1, 2024
 *      Author: thebu
 */

#ifndef INC_MYRTOS_H_
#define INC_MYRTOS_H_


void RTOS_Stats_Timer_Init(void);

void RTOS_ISR_setPriority(uint32_t IRQn);

void TIM2_IRQHandler(void);

void configureTimerForRunTimeStats(void);

unsigned long getRunTimeCounterValue(void);

void safexTaskCreate(
    TaskFunction_t a1,      // Function pointer to the task code
    const char * const a2,        // Descriptive name for the task (optional)
    const uint32_t a3,      // Depth of the task's stack (in words)
    void * const a4,       // Optional parameter to pass to the task
    UBaseType_t a5,          // Task priority (e.g., tskIDLE_PRIORITY)
    StackType_t * const a6, // Optional: Buffer to use for the task's stack
    StaticTask_t * const a7   // Optional: Buffer for task static allocation  (advanced)
);

#endif /* INC_MYRTOS_H_ */
