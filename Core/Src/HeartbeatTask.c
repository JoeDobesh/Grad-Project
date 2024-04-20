/*
 * HeartbeatTask.c
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#include "HeartbeatTask.h"
#include "Globals.h"
#include "main.h"
#include "KernalThread.h"

#define FLASH_TIME_MS 1000

uint32_t heartbeatStack[HEARTBEAT_STACK_SIZE];
uint32_t heartbeatStack2[HEARTBEAT_STACK_SIZE_2];
STATES heartbeatTaskState1;
STATES heartbeatTaskState2;
uint32_t timeout1;
uint32_t timeout2;

//*****************************************************************************
// GetTicks
//*****************************************************************************
//static uint32_t GetTicks(void)
//{
//	uint32_t ticks;

	//__disable_irq();
//	ticks = HAL_GetTick();
	//__enable_irq();

//	return ticks;
//}

//*****************************************************************************
// HeartbeatTask
//*****************************************************************************
void HeartbeatTask(void)
{
	heartbeatTaskState1 = stateZero;
	timeout1 = HAL_GetTick() + FLASH_TIME_MS;
	HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 0);
	while(TRUE)
	{
		switch(heartbeatTaskState1)
		{
		case stateZero:
			if (timeout1 < HAL_GetTick() )
			{
				HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 1);
				timeout1 = HAL_GetTick() + FLASH_TIME_MS;
				heartbeatTaskState1++;
			}
			break;
		case stateOne:
			if (timeout1 <  HAL_GetTick())
			{
				HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 0);
				timeout1 = HAL_GetTick() + FLASH_TIME_MS;
				heartbeatTaskState1--;
			}
			break;
		default:
			heartbeatTaskState1 = stateZero;
			break;
		}
	}
}

//*****************************************************************************
// HeartbeatTask2
//*****************************************************************************
void HeartbeatTask2(void)
{
	heartbeatTaskState2 = stateZero;
	timeout2 = HAL_GetTick() + FLASH_TIME_MS;
	HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Blue_Test_LED_Pin, 0);
	while(1)
	{
		switch(heartbeatTaskState2)
		{
		case stateZero:
			if (timeout2 < HAL_GetTick() )
			{
				HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Blue_Test_LED_Pin, 1);
				timeout2 = HAL_GetTick() + FLASH_TIME_MS;
				heartbeatTaskState2++;
			}
			break;
		case stateOne:
			if (timeout2 <  HAL_GetTick())
			{
				HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Blue_Test_LED_Pin, 0);
				timeout2 = HAL_GetTick() + FLASH_TIME_MS;
				heartbeatTaskState2--;
			}
			break;
		default:
			heartbeatTaskState2 = stateZero;
			break;
		}
	}
}

// EOF

//lines 37
