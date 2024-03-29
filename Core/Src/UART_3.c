/*
 * UART_3.c
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#include "UART_3.h"

extern UART_HandleTypeDef huart3;

//*****************************************************************************
// UART_3_Init
//*****************************************************************************
void UART_3_Init()
{
	printf("UART_3_Init - Passed\n");
}

//*****************************************************************************
// UART_3_Task
//*****************************************************************************
void UART_3_Task(uint8_t ch)
{
	HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
}

//*****************************************************************************
// UART_3_SendString
//*****************************************************************************
BOOL UART_3_SendString(char * data,  size_t size)
{
	uint8_t index;
	uint8_t * ch = (uint8_t *)data;
	if(size > 256)
	{
		return FALSE;
	}
	for(index = 0; index < size; index++)
	{
		HAL_UART_Transmit(&huart3, (uint8_t *)ch, 1, HAL_MAX_DELAY);
		ch++;
	}

	return TRUE;
}

//*****************************************************************************
// UART_3_TxInterrupt
//*****************************************************************************
void UART_3_TxInterrupt(UART_HandleTypeDef *huart)
{

}

//*****************************************************************************
// UART_3_RxInterrupt
//*****************************************************************************
void UART_3_RxInterrupt(UART_HandleTypeDef *huart)
{

}

// EOF
//lines 35
