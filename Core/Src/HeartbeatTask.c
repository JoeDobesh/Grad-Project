/*
 * HeartbeatTask.c
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#include "HeartbeatTask.h"
#include "Globals.h"
#include "main.h"

#define FLASH_TIME 1000

static STATES heartbeatTaskState = stateZero;
static uint32_t timeout = 0;

//*****************************************************************************
// HeartbeatTask
//*****************************************************************************
void HeartbeatTask(void)
{
	switch(heartbeatTaskState)
	{
	case stateZero:
		HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 1);
		timeout = HAL_GetTick() + FLASH_TIME;
		heartbeatTaskState++;
		break;
	case stateOne:
		if ( timeout <  HAL_GetTick())
		{
			HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 0);
			timeout = HAL_GetTick() + FLASH_TIME;
			heartbeatTaskState++;
		}
		break;
	case stateTwo:
		if ( timeout <  HAL_GetTick())
		{
			HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 1);
			timeout = HAL_GetTick() + FLASH_TIME;
			heartbeatTaskState--;
		}
		break;
	default:
		heartbeatTaskState = stateZero;
		break;
	}
}

// EOF
//lines 40
