/*
 * UART_3.h
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#ifndef INC_UART_3_H_
#define INC_UART_3_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

void UART_3_Init(void);
void UART_3_Task(uint8_t ch);
BOOL UART_3_SendString(char *,  size_t);
void UART_3_TxInterrupt(UART_HandleTypeDef *huart);
void UART_3_RxInterrupt(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* INC_UART_3_H_ */

// EOF
//lines 6
