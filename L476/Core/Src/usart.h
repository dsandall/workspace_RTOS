/*
 * usart.h
 *
 *  Created on: Apr 22, 2024
 *      Author: thebu
 */

#ifndef SRC_USART_H_
#define SRC_USART_H_

void USART_init(void);
void USART_Print(const char* message);
uint32_t convertArrayToInt(char* array);


#endif /* SRC_USART_H_ */
