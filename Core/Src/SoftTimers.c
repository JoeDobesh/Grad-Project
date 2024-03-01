/*
 * SoftTimers.c
 *
 *  Created on: Feb 15, 2023
 *      Author: jdobesh
 */

#include "SoftTimers.h"
#include "main.h"
#include "KernalThread.h"
#include "UART_3.h"

#define TIMER_SIZE 8

extern UART_HandleTypeDef huart3;

static TIMER softTimers[TIMER_SIZE];
static BOOL testPassed;
static int testState = 0;

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
	testState = 0;
	printf("SoftTimerInit - Passed\n");
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
			return (i + 1);
		}
	}
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
	else
	{
		softTimers[index-1].params.countTime_ms = (time < 10)? 1: (time/10);
	}
	HAL_NVIC_DisableIRQ(TIM3_IRQn);
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
// SoftTimerBlinkBlueLED
//*****************************************************************************
static void SoftTimersBlinkBlueLED(void)
{
	HAL_GPIO_TogglePin(GPIOB, Blue_Test_LED_Pin);
}

//*****************************************************************************
// SoftTimerOneShotTest
//*****************************************************************************
static void SoftTimersOneShotTest(void)
{
	printf("Oneshot Test Passed\n");
	testPassed = TRUE;
}

//*****************************************************************************
// StartTimerTest
//*****************************************************************************
void StartTimerTest(void)
{
	testState = 1;
}

//*****************************************************************************
// TimerTestComplete
//*****************************************************************************
BOOL TimerTestComplete(void)
{
	if(testState == 0)
	{
		return TRUE;
	}

	return FALSE;
}

//*****************************************************************************
// TimerTask
//*****************************************************************************
void TimerTask(void)
{
	uint32_t status;
	TIMER_PARAMS params;
	static uint8_t index;
	char ch;

	switch(testState)
	{
	case 0: //Idle
		break;
	case 1:
		testPassed = FALSE;
		params.callbackFunctionPtr = SoftTimersBlinkBlueLED;
		params.countTime_ms = 1000;
		params.timerType = CONTINUOUS;
		index = RegisterTimer(params);
		if ( index == 0 )
		{
			printf("\nNo Timers Available. Exiting Test\n");
			testState = 50;
			break;
		}
		if(StartTimer(index, 1000) == FALSE)
		{
			printf("Something Went Wrong with StartTimer Index\n");
			testState = 50;
			break;
		}
		printf("Checking 1 Second Continuous Timer\n");
		printf("Blue LED Should Be Blinking in 1 Second Intervals\n");
		printf("Press Any Key To Move On To The Next Text\n");
		testState++;
		break;
	case 2:
		status = HAL_UART_Receive(&huart3, (uint8_t *)&ch, 1, 10); //HAL_MAX_DELAY);
		if(status == HAL_UART_ERROR_NONE)
		{
			if ( ReleaseTimer(index) == FALSE )
			{
				printf("Error During ReleaseTimer\n");
			}
			testState++;
		}
		break;
	case 3:
		params.callbackFunctionPtr = SoftTimersOneShotTest;
		params.countTime_ms = 1000;
		params.timerType = ONE_SHOT;
		index = RegisterTimer(params);
		if ( index == 0 )
		{
			printf("\nNo Timers Available. Exiting Test\n");
			testState = 50;
			break;
		}
		if(StartTimer(index, 1000) == FALSE)
		{
			printf("Something Went Wrong with StartTimer Index\n");
			testState = 50;
			break;
		}
		printf("Checking 1 Second Oneshot Timer\n");
		printf("Press Any Key To End Text\n");
		testState++;
		break;
	case 4:
		if ( testPassed == TRUE )
		{
			printf("Looks Like All Tests Passed\n");
			if ( ReleaseTimer(index) == FALSE )
			{
				printf("Error During ReleaseTimer\n");
			}
			testState++;
		}
		status = HAL_UART_Receive(&huart3, (uint8_t *)&ch, 1, 10); //HAL_MAX_DELAY);
		if(status == HAL_UART_ERROR_NONE)
		{
			printf("Test Prematurely Ended\n");
			if ( ReleaseTimer(index) == FALSE )
			{
				printf("Error During ReleaseTimer\n");
			}
			testState++;
		}
		break;
	default:
		testState = 0;
		break;
	}
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
