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

static uint32_t pulseWidth = 1;
//*****************************************************************************
// PowerControlInit
//*****************************************************************************
void PowerControlInit(void)
{
	pulseWidth = 1;
	__HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, pulseWidth);
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
// PowerControlTask
//*****************************************************************************
void PowerControlTask(void)
{
	char ch = '\0';
	uint32_t retVal;

	printf("Power Control: < = decrease, > = increase, 0 = exit\n");
	while(ch != '0')
	{
		retVal = HAL_UART_Receive(&huart3, (uint8_t *)&ch, 1, 10); //HAL_MAX_DELAY);
		if(retVal == HAL_UART_ERROR_NONE)
		{
			if(ch == '<')
			{
				PowerControlDecrament();
				printf("Pulsewidth: %lu\n", pulseWidth);
			}
			if(ch == '>')
			{
				PowerControlIncrament();
				printf("Pulsewidth: %lu\n", pulseWidth);
			}
		}
	}
}

//EOF
//lines 100
