/*
 * SwitchPowerTask.c
 *
 *  Created on: Aug 17, 2023
 *      Author: joe.dobesh
 */
#include "UserApps/SwitchPowerTask.h"
#include "KernalThread.h"
#include "SoftTimers.h"
#include "Modbus.h"

#define COMMON_SENSOR 0x10
#define MAIN_SENSOR 0x20
#define SIDE_SENSOR	0x40

uint32_t crossoverStack[CROSSOVER_STACK_SIZE];

static STATES msgState;
static STATES state;

typedef enum _POLARITIES_
{
	POSITIVE = 0,
	NEGATIVE
}POLARITIES;

static POLARITIES mainLine;
static POLARITIES loopLine;
static SWITCH_STATES switchState;
static BOOL sensorCommon;
static BOOL sensorMain;
static BOOL sensorLoop;
static uint32_t timerId;

//*****************************************************************************
// SetLoopPolarity
//*****************************************************************************
static void SetLoopPolarity(POLARITIES pol)
{
	if(loopLine == pol)
	{
		return;
	}
	//TODO: Switch polarity
	loopLine = pol;
}

//*****************************************************************************
// SetMainPolarity
//*****************************************************************************
static void SetMainPolarity(POLARITIES pol)
{
	if(mainLine == pol)
	{
		return;
	}
	//TODO: Switch polarity
	loopLine = pol;
}

//*****************************************************************************
// SwitchPowerInit
//*****************************************************************************
static void SwitchPowerInit(void)
{
	TIMER_PARAMS timerParams;

	timerParams.callbackFunctionPtr = NULL;
	timerParams.countTime_ms = SCAN_TIMEOUT;
	timerParams.timerType = ONE_SHOT;
	timerId = RegisterTimer(timerParams);
	if(timerId == 0)
	{
		printf("Switch Power Init - Failed: RegisterTimer");
		return;
	}
	msgState     = stateZero;
	state        = stateZero;
	sensorCommon = FALSE;
	sensorMain   = FALSE;
	sensorLoop   = FALSE;
	OpenSwitch();
	SetMainPolarity(POSITIVE);
	SetLoopPolarity(POSITIVE);
	printf("Switch Power Init - Passed\n");
}

//*****************************************************************************
// CloseSwitch
//*****************************************************************************
void CloseSwitch(void)
{
	if(sensorCommon == TRUE && sensorMain == TRUE)
	{
		return;
	}
	//TODO: Close switch
	switchState = SWITCH_CLOSED;
}

//*****************************************************************************
// OpenSwitch
//*****************************************************************************
void OpenSwitch(void)
{
	if(sensorCommon == TRUE && sensorLoop == TRUE)
	{
		return;
	}
	//TODO: Open switch
	switchState = SWITCH_OPEN;
}

//*****************************************************************************
// GetSwitchStatus
//*****************************************************************************
SWITCH_STATES GetSwitchStatus(void)
{
	return switchState;
}

//*****************************************************************************
// GetSensorStatus
//*****************************************************************************
static void GetSensorStatus(void)
{
	uint16_t data = ModbusGetCrossoverSensors();
	if(data & 0x0010)
	{
		sensorCommon = TRUE;
	}
	else
	{
		sensorCommon = FALSE;
	}
	if(data & 0x0020)
	{
		sensorMain = TRUE;
	}
	else
	{
		sensorMain = FALSE;
	}
	if(data & 0x0040)
	{
		sensorLoop = TRUE;
	}
	else
	{
		sensorLoop = FALSE;
	}
}

//*****************************************************************************
// SwitchPowerTask
//*****************************************************************************
void SwitchPowerTask(void)
{
	BOOL exit = FALSE;

	SwitchPowerInit();
	while(exit == FALSE)
	{
		GetSensorStatus();
		if(switchState == SWITCH_OPEN)
		{
			if((sensorMain == TRUE || sensorCommon == TRUE) && (mainLine == POSITIVE && loopLine == NEGATIVE))
			{
				SetLoopPolarity(POSITIVE);
			}
		}
		else //switchState == CLOSED
		{
			if(mainLine == POSITIVE)
			{
				if(loopLine == POSITIVE)
				{
					if(sensorCommon == TRUE && sensorLoop == FALSE)
					{
						SetLoopPolarity(NEGATIVE);
					}
					else if(sensorCommon == FALSE && sensorLoop == TRUE)
					{
						SetMainPolarity(NEGATIVE);
					}
				}
				else
				{
					if(sensorCommon == FALSE && sensorLoop == TRUE)
					{
						SetMainPolarity(NEGATIVE);
					}
				}
			}
		}
	}
}

//EOF

//lines 180
