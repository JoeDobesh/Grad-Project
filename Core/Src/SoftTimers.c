/*
 * SoftTimers.c
 *
 *  Created on: Feb 15, 2023
 *      Author: jdobesh
 */

#include "SoftTimers.h"
#include "main.h"
#include "KernalThread.h"
#include "Mutex.h"

#define TIMER_SIZE 8
#define TIMER_STACK_SIZE 1000

uint32_t softTimerStack[TIMER_STACK_SIZE];

static TIMER softTimers[TIMER_SIZE];

//*****************************************************************************
// Constructor
//*****************************************************************************
void SoftTimerInit(void)
{
	for(uint8_t i = 0; i < TIMER_SIZE; i++)
	{
		softTimers[i].registered = FALSE;
		softTimers[i].active = FALSE;
		softTimers[i].counter = 0;
	}
}

//*****************************************************************************
// TimerParametersInit
//*****************************************************************************
void TimerParametersInit(TIMER_PARAMS* parameters)
{
	parameters->timerType = ONE_SHOT;
	parameters->countTime_ms = 0;
	parameters->callbackFunctionPtr = NULL;
}

//*****************************************************************************
// RegisterTimer
//*****************************************************************************
uint8_t RegisterTimer(TIMER_PARAMS parameters)
{
	HAL_NVIC_DisableIRQ(TIM3_IRQn);
	for(uint8_t i = 0; i < TIMER_SIZE; i++)
	{
		if(softTimers[i].registered == FALSE)
		{
			softTimers[i].params.countTime_ms = (parameters.countTime_ms < 10)? 1: (parameters.countTime_ms/10);
			softTimers[i].params.timerType = parameters.timerType;
			softTimers[i].params.callbackFunctionPtr = parameters.callbackFunctionPtr;
			softTimers[i].counter = 0;
			softTimers[i].active = FALSE;
			softTimers[i].registered = TRUE;
			HAL_NVIC_EnableIRQ(TIM3_IRQn);
			return (i + 1);
		}
	}
	HAL_NVIC_EnableIRQ(TIM3_IRQn);
	printf ("SoftTimers::GetTimer - Ran Out Of Timers\r\n");

	return 0;
}

//*****************************************************************************
// StartTimer
//*****************************************************************************
BOOL StartTimer(uint8_t index, uint64_t time)
{
	if(index == 0)
	{
		return FALSE;
	}
	if(index > TIMER_SIZE)
	{
		return FALSE;
	}
	if(softTimers[index-1].registered == FALSE)
	{
		return FALSE;
	}
	if ( time == 0 )
	{
		return TRUE;
	}
	HAL_NVIC_DisableIRQ(TIM3_IRQn);
	softTimers[index-1].params.countTime_ms = (time < 10)? 1: (time/10);
	//I did this so the interrupt would not fire so often
	softTimers[index-1].counter = softTimers[index-1].params.countTime_ms;
	softTimers[index-1].active = TRUE;
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	return TRUE;
}

//*****************************************************************************
// CheckTimer
//*****************************************************************************
BOOL CheckTimer(uint32_t index)
{
	if(index == 0)
	{
		return FALSE;
	}
	if(index > TIMER_SIZE)
	{
		return FALSE;
	}
	if(softTimers[index-1].registered == FALSE)
	{
		return FALSE;
	}
	//if(softTimers[index-1].active == FALSE)
	//{
	//	return FALSE;
	//}
	if ( softTimers[index-1].counter == 0 )
	{
		return TRUE;
	}

	return FALSE;
}

//*****************************************************************************
// ReleaseTimer
//*****************************************************************************
BOOL ReleaseTimer(uint32_t index)
{
	if(index == 0)
	{
		return FALSE;
	}
	if(index > TIMER_SIZE)
	{
		return FALSE;
	}
	HAL_NVIC_DisableIRQ(TIM3_IRQn);
	softTimers[index-1].registered = FALSE;
	HAL_NVIC_EnableIRQ(TIM3_IRQn);

	return TRUE;
}

//*****************************************************************************
// TimerInterrrupt
//*****************************************************************************
void TimerInterrupt(void)
{
	for(uint8_t i = 0; i < TIMER_SIZE; i++)
	{
		if(softTimers[i].registered == TRUE)
		{
			if(softTimers[i].active == TRUE)
			{
				softTimers[i].counter--;
				if(softTimers[i].counter == 0)
				{
					if(softTimers[i].params.callbackFunctionPtr != NULL)
					{
						softTimers[i].params.callbackFunctionPtr();
					}
					if(softTimers[i].params.timerType == ONE_SHOT)
					{
						softTimers[i].active = FALSE;
					}
					else
					{
						softTimers[i].counter = softTimers[i].params.countTime_ms;
					}
				}
			}
		}
	}
}

// EOF
//lines 250
