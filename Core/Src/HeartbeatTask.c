/*
 * HeartbeatTask.c
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#include "HeartbeatTask.h"
#include "Globals.h"
#include "main.h"
#include "SoftTimers.h"
#include "Mutex.h"

#define FLASH_TIME_MS 1000

uint32_t heartbeatStack1[HEARTBEAT_STACK_SIZE_1];
uint32_t heartbeatStack2[HEARTBEAT_STACK_SIZE_2];

PCB heartbeatPCB1;
PCB heartbeatPCB2;

//*****************************************************************************
// HeartbeatTask1
//*****************************************************************************
void HeartbeatTask1(void)
{
	static uint32_t timeout1;
	uint32_t tickCount;

	HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin, 1);
	timeout1 = HAL_GetTick() + FLASH_TIME_MS;
	while(TRUE)
	{
		tickCount = HAL_GetTick();
		if (tickCount > timeout1)
		{
			timeout1 = tickCount + FLASH_TIME_MS;
			HAL_GPIO_TogglePin(Heartbeat_LED_GPIO_Port, Heartbeat_LED_Pin);
		}
	}
}

//*****************************************************************************
// HeartbeatTask2
//*****************************************************************************
void HeartbeatTask2(void)
{
	static uint32_t timeout2;
	uint32_t tickCount;

	HAL_GPIO_WritePin(Heartbeat_LED_GPIO_Port, Blue_Test_LED_Pin, 1);
	timeout2 = HAL_GetTick() + FLASH_TIME_MS;
	while(TRUE)
	{
		tickCount = HAL_GetTick();
		if (tickCount > timeout2)
		{
			timeout2 = tickCount + FLASH_TIME_MS;
			HAL_GPIO_TogglePin(Heartbeat_LED_GPIO_Port, Blue_Test_LED_Pin);
		}
	}
}

// EOF

//lines 37
