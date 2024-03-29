/*
 * HeartbeatTask.c
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#include "HeartbeatTask.h"
#include "Globals.h"
#include "main.h"

#define FLASH_TIME_MS 1000

uint32_t heartbeatStack[HEARTBEAT_STACK_SIZE];

typedef struct __HEARTBEAT_MEMORY__
{
	uint32_t timeout;
	STATES heartbeatTaskState;
	BOOL exit;
}HEARTBEAT_MEMORY;

//*****************************************************************************
// HeartbeatTask
//*****************************************************************************
void HeartbeatTask(void)
{
	HEARTBEAT_MEMORY * heartbeatMemoryPtr;

	heartbeatMemoryPtr = malloc(sizeof(HEARTBEAT_MEMORY));
	if(heartbeatMemoryPtr == NULL)
	{
		return;
	}
	heartbeatMemoryPtr->exit = FALSE;
	heartbeatMemoryPtr->heartbeatTaskState = stateZero;
	heartbeatMemoryPtr->timeout = HAL_GetTick() + FLASH_TIME_MS;
	HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 0);
	while(heartbeatMemoryPtr->exit == FALSE)
	{
		switch(heartbeatMemoryPtr->heartbeatTaskState)
		{
		case stateZero:
			if ( heartbeatMemoryPtr->timeout < HAL_GetTick() )
			{
				HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 1);
				heartbeatMemoryPtr->timeout = HAL_GetTick() + FLASH_TIME_MS;
				heartbeatMemoryPtr->heartbeatTaskState++;
			}
			break;
		case stateOne:
			if ( heartbeatMemoryPtr->timeout <  HAL_GetTick())
			{
				HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 0);
				heartbeatMemoryPtr->timeout = HAL_GetTick() + FLASH_TIME_MS;
				heartbeatMemoryPtr->heartbeatTaskState--;
			}
			break;
		default:
			heartbeatMemoryPtr->heartbeatTaskState = stateZero;
			break;
		}
	}
	free(heartbeatMemoryPtr);
}

// EOF
//lines 40
