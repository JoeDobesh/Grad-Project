/*
 * RS485.c
 *
 *  Created on: Aug 17, 2023
 *      Author: joe.dobesh
 */

#include "RS485.h"

#define CIRCULAR_BUFFER_SIZE 64

extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart3;

static uint8_t circularDMABuffer[CIRCULAR_BUFFER_SIZE];
static uint8_t circularRxBuffer[CIRCULAR_BUFFER_SIZE];
static uint16_t rxBuffPushIndex;
static uint16_t rxBuffPullIndex;
static uint8_t testState = 0;

//*****************************************************************************
// RS485Init
//*****************************************************************************
void RS485Init(void)
{
	HAL_StatusTypeDef retVal;

	rxBuffPushIndex = 0;
	rxBuffPullIndex = 0;
	HAL_GPIO_WritePin(RS485_REN_DEN_GPIO_Port, RS485_REN_DEN_Pin, GPIO_PIN_RESET);
	testState = 0;
	retVal = HAL_UARTEx_ReceiveToIdle_DMA(&huart4, circularDMABuffer, CIRCULAR_BUFFER_SIZE);
	switch(retVal)
	{
		case HAL_OK:
			printf("RS485Init - Passed\n");
			break;
		case HAL_ERROR:
		case HAL_BUSY:
		case HAL_TIMEOUT:
		default:
			printf("RS485Init - Failed\n");
			break;
	}
}

//*****************************************************************************
// US485SendString
//*****************************************************************************
BOOL RS485SendString(uint8_t * data,  size_t size)
{
	HAL_StatusTypeDef retVal;
	uint8_t * bufferPtr = data;
	if(size > 256)
	{
		return FALSE;
	}
	HAL_GPIO_WritePin(RS485_REN_DEN_GPIO_Port, RS485_REN_DEN_Pin, GPIO_PIN_SET);
	retVal = HAL_UART_Transmit_IT(&huart4, bufferPtr, size);
	switch(retVal)
	{
		case HAL_OK:
			return TRUE;
			break;
		case HAL_ERROR:
		case HAL_BUSY:
		case HAL_TIMEOUT:
		default:
			HAL_GPIO_WritePin(RS485_REN_DEN_GPIO_Port, RS485_REN_DEN_Pin, GPIO_PIN_RESET);
			break;
	}

	return FALSE;
}

//*****************************************************************************
// RS485GetString
//*****************************************************************************
BOOL RS485GetString(uint8_t * data, size_t size)
{
	uint16_t counter = 0;

	if(size >= CIRCULAR_BUFFER_SIZE || size <= 0)
	{
		return FALSE;
	}
	if(rxBuffPushIndex == rxBuffPullIndex)
	{
		return FALSE;
	}
	while(counter < size && rxBuffPushIndex != rxBuffPullIndex)
	{
		data[counter] = circularRxBuffer[rxBuffPullIndex];
		counter++;
		rxBuffPullIndex++;
		rxBuffPullIndex = (rxBuffPullIndex >= CIRCULAR_BUFFER_SIZE)? 0: rxBuffPullIndex;
	}

	return TRUE;
}

//*****************************************************************************
// RS485TxInterrupt
//*****************************************************************************
void RS485TxInterrupt(void)
{
	HAL_GPIO_WritePin(RS485_REN_DEN_GPIO_Port, RS485_REN_DEN_Pin, GPIO_PIN_RESET);
}

//*****************************************************************************
// RS485RxInterrupt
//*****************************************************************************
void RS485RxInterrupt(uint16_t length)
{
	if (rxBuffPushIndex == length)
	{
		return;
	}
	while(rxBuffPushIndex != length)
	{
		circularRxBuffer[rxBuffPushIndex] = circularDMABuffer[rxBuffPushIndex];
		rxBuffPushIndex++;
		rxBuffPushIndex = (rxBuffPushIndex >= CIRCULAR_BUFFER_SIZE)? 0: rxBuffPushIndex;
	}
}

//*****************************************************************************
// RS485RxInterrupt
//*****************************************************************************
/*
void RS485RxInterrupt(void)
{
	HAL_StatusTypeDef retVal;
	uint8_t ch;

	retVal = HAL_UART_Receive_IT(&huart4, &ch, 1);
	switch(retVal)
	{
		case HAL_OK:
			circularRxBuffer[rxBuffPushIndex] = ch;
			rxBuffPushIndex++;
			rxBuffPushIndex = (rxBuffPushIndex >= CIRCULAR_BUFFER_SIZE)? 0: rxBuffPushIndex;
			break;
		case HAL_ERROR:
		case HAL_BUSY:
		case HAL_TIMEOUT:
		default:
			break;
	}
}

void RS485RxInterrupt(void)
{
	HAL_StatusTypeDef retVal;
	uint8_t ch;

	while(1)
	{
		retVal = HAL_UART_Receive_IT(&huart4, &ch, 1);
		switch(retVal)
		{
			case HAL_OK:
				circularRxBuffer[rxBuffPushIndex] = ch;
				rxBuffPushIndex++;
				rxBuffPushIndex = (rxBuffPushIndex >= CIRCULAR_BUFFER_SIZE)? 0: rxBuffPushIndex;
				break;
			case HAL_ERROR:
			case HAL_BUSY:
			case HAL_TIMEOUT:
			default:
				return;
				break;
		}
	}
}
*/

//*****************************************************************************
// UART_3_SendString
//*****************************************************************************
static BOOL UART_3_SendString(char * data,  size_t size)
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
// RS485Test
//*****************************************************************************
void RS485Test(void)
{
	HAL_StatusTypeDef status;
	char str1[] = "RS485>>";
	uint8_t ch;
	size_t size = sizeof(str1);
	uint8_t inputBuffer[32];
	static uint32_t inputBuffCounter;
	BOOL exit = FALSE;

	testState = 0;
	while(exit == FALSE)
	{
		switch(testState)
		{
		case 0:
			RS485Init();
			printf("Type something to send\n");
			UART_3_SendString(str1,  size);
			inputBuffCounter = 0;
			testState++;
			break;
		case 1:
			status = HAL_UART_Receive(&huart3, &ch, 1, 10);
			if(status == HAL_OK)
			{
				if(ch == '\n')
				{
					inputBuffer[inputBuffCounter++] = '\0';
					HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
					RS485SendString(inputBuffer,  inputBuffCounter);
					testState++;
				}
				else
				{
					inputBuffer[inputBuffCounter++] = ch;
					inputBuffCounter = (inputBuffCounter >= 32)? (32 - 1): inputBuffCounter;
					HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				}
			}
			break;
		case 2:
			if(RS485GetString(&ch, 1) == TRUE)
			{
				HAL_UART_Transmit(&huart3, &ch, 1, HAL_MAX_DELAY);
			}
			status = HAL_UART_Receive(&huart3, &ch, 1, 10);
			if(status == HAL_OK)
			{
				if(ch == '0')
				{
					exit = TRUE;
				}
				testState++;
			}
			break;
		default:
			testState = 0;
			break;
		}
	}
}

// EOF
//lines 200
