/*
 * general.h
 *
 *  Created on: Apr 22, 2024
 *      Author: thebu
 */

#ifndef INC_GENERAL_H_
#define INC_GENERAL_H_

void PortC_Init(void);
void PortA_Init(void);

void MSI_Overclock(void);

int _write(int file, char *ptr, int len);



#endif /* INC_GENERAL_H_ */
