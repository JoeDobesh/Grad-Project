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

//*****************************************************************************
// HeartbeatTask
//*****************************************************************************
void HeartbeatTask(void)
{
	BOOL exit = FALSE;
	STATES heartbeatTaskState = stateZero;
	uint32_t timeout = HAL_GetTick() + FLASH_TIME_MS;

	HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 0);
	while(exit == FALSE)
	{
		switch(heartbeatTaskState)
		{
		case stateZero:
			if (timeout < HAL_GetTick() )
			{
				HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 1);
				timeout = HAL_GetTick() + FLASH_TIME_MS;
				heartbeatTaskState++;
			}
			break;
		case stateOne:
			if (timeout <  HAL_GetTick())
			{
				HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 0);
				timeout = HAL_GetTick() + FLASH_TIME_MS;
				heartbeatTaskState--;
			}
			break;
		default:
			heartbeatTaskState = stateZero;
			break;
		}
	}
}

// EOF

//lines 37
