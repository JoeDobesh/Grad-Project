/*
 * SwitchPowerTask.c
 *
 *  Created on: Aug 17, 2023
 *      Author: joe.dobesh
 */
#include "UserApps/SwitchPowerTask.h"
#include "KernalThread.h"
#include "Modbus.h"
#include "FIFO.h"
#include "MailBag.h"
#include "Mutex.h"

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
	msgState     = stateZero;
	state        = stateZero;
	sensorCommon = FALSE;
	sensorMain   = FALSE;
	sensorLoop   = FALSE;
	OpenSwitch();
	SetMainPolarity(POSITIVE);
	SetLoopPolarity(POSITIVE);
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
// SwitchPowerTask
//*****************************************************************************
void SwitchPowerTask(void)
{
	static uint16_t myData;
	static BOOL error;

	SwitchPowerInit();
	if(RegisterFIFOInput(CROSSOVER_OUT_ID) == FALSE)
	{
		printf("Crossover Task Init - Failed: RegisterFIFOInput");
		while(TRUE){;}
	}
	while(TRUE)
	{
		error = GetFIFOData(CROSSOVER_IN_ID, &myData);
		if(error == TRUE)
		{
			//GetSensorStatus();
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
}

//EOF

//lines 180
