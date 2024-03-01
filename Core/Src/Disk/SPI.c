/*
 * SPI.c
 *
 *  Created on: Aug 18, 2023
 *      Author: joe.dobesh
 */

#include "SPI.h"

extern SPI_HandleTypeDef hspi1;

//*****************************************************************************
// SPI_Init
//*****************************************************************************
BOOL SPI_Init(void)
{
	SPI_CS_High();
	printf("SPI_Init Passed\n");

	return TRUE;
}

//*****************************************************************************
// SPI_CS_High
//*****************************************************************************
void SPI_CS_High(void)
{
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);
}

//*****************************************************************************
// SPI_CS_Low
//*****************************************************************************
void SPI_CS_Low(void)
{
	HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
}

//*****************************************************************************
// SPI_WriteRead
//*****************************************************************************
BOOL SPI_WriteRead(uint8_t * data, uint16_t size)
{
	BOOL retVal;
	uint8_t temp[1];

	temp[0] = 0xFF;
	HAL_StatusTypeDef status = HAL_SPI_TransmitReceive(&hspi1, temp, data, size, 2000);
	switch ( status )
	{
	case HAL_OK:
		retVal = TRUE;
		break;
	case HAL_ERROR:
		retVal = FALSE;
		break;
	case HAL_BUSY:
		retVal = FALSE;
		break;
	case HAL_TIMEOUT:
		retVal = FALSE;
		break;
	default:
		retVal = FALSE;
		break;
	}

	return retVal;
}

//*****************************************************************************
// SPI_Read
//*****************************************************************************
BOOL SPI_Read(uint8_t * data, uint16_t size)
{
	BOOL retVal;

	HAL_StatusTypeDef status = HAL_SPI_Receive(&hspi1, data, size, 2000);
	switch ( status )
	{
	case HAL_OK:
		retVal = TRUE;
		break;
	case HAL_ERROR:
		retVal = FALSE;
		break;
	case HAL_BUSY:
		retVal = FALSE;
		break;
	case HAL_TIMEOUT:
		retVal = FALSE;
		break;
	default:
		retVal = FALSE;
		break;
	}

	return retVal;
}

//*****************************************************************************
// SPI_Write
//*****************************************************************************
BOOL SPI_Write(uint8_t * data, uint16_t size)
{
	BOOL retVal;
	HAL_StatusTypeDef status = HAL_SPI_Transmit(&hspi1, data, size, 2000);
	switch ( status )
	{
	case HAL_OK:
		retVal = TRUE;
		break;
	case HAL_ERROR:
		retVal = FALSE;
		break;
	case HAL_BUSY:
		retVal = FALSE;
		break;
	case HAL_TIMEOUT:
		retVal = FALSE;
		break;
	default:
		retVal = FALSE;
		break;
	}

	return retVal;
}

//EOF

//lines 100
