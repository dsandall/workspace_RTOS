/*
 * usart.c
 *
 *  Created on: Apr 22, 2024
 *      Author: thebu
 */

#include "main.h"
#include <stdarg.h>


#define F_CLK 32000000 	// bus clock is 4 MHz
#define BAUD_RATE 460800




void USART_init(void){
	// configure GPIO pins for USART2 (PA2, PA3) follow order of configuring registers
	// AFR, OTYPER, PUPDR, OSPEEDR, MODDER otherwise a glitch is created on the output pin
	RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN);
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL2 | GPIO_AFRL_AFSEL3);		// mask AF selection
	GPIOA->AFR[0] |= ((7 << GPIO_AFRL_AFSEL2_Pos ) |				// select USART2 (AF7)
				   (7 << GPIO_AFRL_AFSEL3_Pos)); 		  	  	// for PA2 and PA3
	GPIOA->OTYPER &= ~(GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);		// push-pull output for PA2, PA3
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);		// no pull ups on PA2, PA3
	GPIOA->OSPEEDR &= ~(GPIO_OSPEEDR_OSPEED2); GPIOA->OSPEEDR |= 0b10<<4;	// high speed
	GPIOA->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);		// enable alternate function
	GPIOA->MODER |= (GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1);    // for PA2 and PA3

	// Configure USART2 connected to the debugger virtual COM port
	RCC->APB1ENR1 |= RCC_APB1ENR1_USART2EN;			// enable USART by turning on system clock
	USART2->CR1 &= ~(USART_CR1_M1 | USART_CR1_M0);	//set data to 8 bits
	USART2->BRR = F_CLK / BAUD_RATE;						// baudrate for 115200
	USART2->CR1 |= USART_CR1_UE;						// enable USART
	USART2->CR1 |= (USART_CR1_TE | USART_CR1_RE);		// enable transmit and receive for USART

	// enable interrupts for USART2 receive
	USART2->CR1 |= USART_CR1_RXNEIE;					// enable RXNE interrupt on USART2
	USART2->ISR &= ~(USART_ISR_RXNE);					// clear interrupt flagwhile (message[i] != 0)

	NVIC->ISER[1] = (1 << (USART2_IRQn & 0x1F));		// enable USART2 ISR
	__enable_irq();
}

void USART_print(const char* message) {
	uint8_t i;
	for(i=0; message[i] != 0; i++){				// check for terminating NULL character
		while(!(USART2->ISR & USART_ISR_TXE));	// wait for transmit buffer to be empty
		USART2->TDR = message[i];				// transmit character to USART
	}
}

void USART_printf(const char* format, ...) {
	va_list args;
	va_start(args, format);

	// Allocate temporary buffer for formatted string
	static char buffer[1024]; // Adjust buffer size as needed
	int formatted_length = vsnprintf(buffer, sizeof(buffer), format, args);

	// Check for potential buffer overflow (optional)
	if (formatted_length >= sizeof(buffer)) {
		// Handle buffer overflow (e.g., print error message)
		printf("first TIMER end value %d\n", TIM5->CNT);
	} else {
		// Print the formatted string
		USART_print(buffer);
	}

	va_end(args);
}

void USART_YX(int line, int column) {
	USART_printf("\x1B[%d;%dH", line, column);  // move cursor to top left
}

void USART_cmd(const char* message) {
	uint8_t i;
	USART_print("\x1B");
	for(i=0; message[i] != 0; i++){				// check for terminating NULL character
		while(!(USART2->ISR & USART_ISR_TXE));	// wait for transmit buffer to be empty
		USART2->TDR = message[i];				// transmit character to USART
	}
}

void USART_putchar(const char c){
	while(!(USART2->ISR & USART_ISR_TXE));	// wait for transmit buffer to be empty
	USART2->TDR = c;				// transmit character to USART
}

void USART_clear(){
	USART_print("\x1B[2J"); // clear screen
	USART_print("\x1B[0m"); // clear attributes
	USART_print("\x1B[H");  // move cursor to top left
}

uint32_t convertArrayToInt(char* array){
	int i = 0;
	uint32_t count = 0;
	while(array[i] != '\0'){
		if ((array[i] >= 0x30) && (array[i] <= 0x39)){
			count = (count * 10) + ((uint32_t)array[i] - 0x30);
			i++;
		} else {return 0;}
	}
	return count;
}

//char userNumberArr[32];
//int userNumPos = 0;
//void USART2_IRQHandler(void){
//	char r;
//
//	if (USART2->ISR & USART_ISR_RXNE){
//		r = USART2->RDR;			// copy received char
//
////		sprintf(buffer, "0x%X\n", r);
////		USART_print(buffer);
//
//		if (r == 0xD){
//			userNumberArr[userNumPos++] = '\0'; //terminate array
//			userNumPos = 0;
//			userNumberMS = convertArrayToInt(userNumberArr);
//			USART_print("\x1B[2J"); // clear screen
//			USART_print("\x1B[0m"); // clear attributes
//			USART_print("\x1B[H");  // move cursor to top left
//			if (userNumberMS == 0){ //error condition
//				USART_print("Error: Please enter only 0-9, number must be at least 1ms");
//				USART_print("\x1B[H");  // move cursor to top left
//				USART_print("\x1B[1B"); // move cursor down 3 lines
//			} else {
//				TIM2->ARR = userNumberMS;
//				TIM2->CNT = 0;
//			}
//
//			sprintf(buffer, "0x%X\n", userNumberMS);
//			USART_print(buffer);
//
//			USART_print(inputMessage);
//
//		} else if (r == 0x7F){
//			USART_print("\x1B[1D"); // move cursor left 1 lines
//			USART_print(" ");
//			USART_print("\x1B[1D"); // move cursor left 1 lines
//
//			userNumberArr[--userNumPos] = 0;
//
//		} else {
//			USART2->TDR = r; //echo received char
//			userNumberArr[userNumPos++] = r;
//
//		}
//
//		USART2->ISR &= ~(USART_ISR_RXNE);	// clear the flag
//	}
//}