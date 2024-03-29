/*
 * PowerControlTask.c
 *
 *  Created on: Jun 7, 2023
 *      Author: joe.dobesh
 */
#include "KernalThread.h"
#include "UserApps/PowerControlTask.h"

extern TIM_HandleTypeDef htim4;
extern UART_HandleTypeDef huart3;

uint32_t speedControlStack[SPEED_CONTROL_STACK_SIZE];

static uint32_t outputVoltage;
static uint32_t pulseWidth = 1;
static uint8_t pcState = 0;
static BOOL status;

//*****************************************************************************
// PowerControlInit
//*****************************************************************************
void PowerControlInit(void)
{
	outputVoltage = 0;
	pulseWidth = 1;
	pcState = 0;
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulseWidth);
	status = FALSE;
	printf("PowerControlInit - Passed\r\n");
}

//*****************************************************************************
// GetPulseWidth
//*****************************************************************************
uint32_t GetPulseWidth(void)
{
	return pulseWidth;
}

//*****************************************************************************
void PowerControlDecrament(void)
{
	pulseWidth = (--pulseWidth < 1)? 1: pulseWidth;
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulseWidth);
}

//*****************************************************************************
// PowerControlIncrament
//*****************************************************************************
void PowerControlIncrament(void)
{
	pulseWidth = (++pulseWidth > 100)? 100: pulseWidth;
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulseWidth);
}

//*****************************************************************************
// PowerControlSet
//*****************************************************************************
void PowerControlSet(uint32_t pulseWidth)
{
	if (pulseWidth > 100)
	{
		pulseWidth = 100;
	}
	if (pulseWidth < 1)
	{
		pulseWidth = 1;
	}
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulseWidth);
}

//*****************************************************************************
//* RunPowerControl
//*****************************************************************************
BOOL RunPowerControl(void)
{
	return TRUE;
}

//*****************************************************************************
// GetPowerControlStatus
//*****************************************************************************
BOOL GetPowerControlStatus(void)
{
	return status;
}

//*****************************************************************************
// PowerControlTask
//*****************************************************************************
void PowerControlTask(void)
{
	static char ch;
	uint32_t retVal;

	switch(pcState)
	{
	case 0:
		printf("Power Control: < = decrease, > = increase, 0 = exit\n");
		pcState++;
		break;
	case 1:
		retVal = HAL_UART_Receive(&huart3, (uint8_t *)&ch, 1, 10); //HAL_MAX_DELAY);
		if(retVal == HAL_UART_ERROR_NONE)
		{
			if(ch == '<')
			{
				PowerControlDecrament();
				printf("Pulsewidth: %d\n", pulseWidth);
			}
			if(ch == '>')
			{
				PowerControlIncrament();
				printf("Pulsewidth: %d\n", pulseWidth);
			}
			if(ch == '0')
			{
				pcState++;
			}
		}
		break;
	default:
		status = FALSE;
		break;
	}
}

//*****************************************************************************
// PowerControlInterrupt
//*****************************************************************************
void PowerControlInterrupt(void)
{
	//static uint32_t pulseWidth = 10;
		//if ( htim == &htim4)
		//{
		//	pulseWidth = (pulseWidth + 1) % 100;
		//	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulseWidth);
		//}


	//const uint32_t step = 2;
	//uint32_t compare = __HAL_TIM_GET_COMPARE(&htim4, TIM_CHANNEL_2) + step;
	//__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, (compare <= 100)? compare: 0 );
}

//EOF
//lines 100
