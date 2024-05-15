/*
 * usart.h
 *
 *  Created on: Apr 22, 2024
 *      Author: thebu
 */

#ifndef SRC_USART_H_
#define SRC_USART_H_

void USART_init(void);
void USART_clear();
void USART_wipe();

void USART_printf(const char* format, ...);
void USART_print(const char* message);

void USART_cmd(const char* message);
void USART_YX(int line, int column);



uint32_t convertArrayToInt(char* array);
void USART_putchar(const char c);


#endif /* SRC_USART_H_ */


