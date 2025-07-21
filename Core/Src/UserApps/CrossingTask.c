/*
 * CrossingTask.c
 *
 *  Created on: Feb 18, 2023
 *      Author: jdobesh
 */

#include "KernalThread.h"
#include "UserApps/CrossingTask.h"
#include "Mutex.h"
#include "Modbus.h"
#include "FIFO.h"
#include "MailBag.h"

#define NUM_CROSSINGS 3
#define FAR_1	0x0001
#define NEAR_2	0x0002
#define NEAR_1	0x0004
#define FAR_2	0x0008
#define CROSSING_SENSOR_MASK 0x000F

uint32_t crossingStack[CROSSING_STACK_SIZE];

typedef struct _CROSSING_INFO_
{
	const char* name;
	uint8_t address;
} CROSSING_INFO;

const char cr5[] = {"County Road 5"};
const char dms[] = {"Denham Main Street"};
const char sh2[] = {"State Highway 2"};

//static CROSSING_INFO crossings[] =
//{
//	{cr5, IO_MODULE_0_ADDRESS},
//	{dms, IO_MODULE_1_ADDRESS},
//	{sh2, IO_MODULE_2_ADDRESS}
//};

static STATES crossingState;
static STATES msgState;
static BOOL gateStatus;
static uint8_t crossingSelect = 0;
static uint16_t sensorData;
//*****************************************************************************
// CrossingInit
//*****************************************************************************
static void CrossingInit(void)
{
	crossingSelect  = 0;
	crossingState   = stateZero;
	msgState        = stateZero;
	gateStatus      = GATE_OPEN;
	sensorData		= 0;
}

//*****************************************************************************
// GetSersorValues
//*****************************************************************************
uint8_t GetSersorValues(void)
{
	return (uint8_t)(sensorData & CROSSING_SENSOR_MASK);
}

//*****************************************************************************
// CrossingTask
//*****************************************************************************
void CrossingTask(void)
{
	static uint16_t myData;
	static uint8_t sensors;
	static BOOL error;

	CrossingInit();
	if(RegisterFIFOInput(CROSSING_OUT_ID) == FALSE)
	{
		printf("Crossing Task Init - Failed: RegisterFIFOInput");
		while(TRUE){;}
	}
	while(TRUE)
	{
		MutexSpinLock(MUTEX_FIFO);
		error = GetFIFOData(CROSSING_IN_ID, &myData);
		MutexRelease(MUTEX_FIFO);
		if(error == TRUE)
		{
			sensorData = myData;
			switch(crossingState)
			{
			case stateZero:
			case stateOne:
				if((sensorData & FAR_1) || (sensorData & NEAR_2))
				{
					//Close Gates
					PutFIFOData(CROSSING_OUT_ID, 0x0002);
					gateStatus    = GATE_CLOSED;
					crossingState = stateTwo;
				}
				else if((sensorData & FAR_2) || (sensorData & NEAR_1))
				{
					//Close Gates
					PutFIFOData(CROSSING_OUT_ID, 0x0002);
					gateStatus    = GATE_CLOSED;
					crossingState = stateThree;
				}
				break;
			case stateTwo:
				if(sensorData & NEAR_2)
				{
					crossingState = stateFour;
				}
				break;
			case stateThree:
				if(sensorData & NEAR_1)
				{
					crossingState = stateFive;
				}
				break;
			case stateFour:
				if((sensorData & NEAR_2) == 0)
				{
					//Open Gates
					PutFIFOData(CROSSING_OUT_ID, 0x0001);
					gateStatus    = GATE_OPEN;
					crossingState = stateSix;
				}
				break;
			case stateFive:
				if((sensorData & NEAR_1) == 0)
				{
					//Open Gates
					PutFIFOData(CROSSING_OUT_ID, 0x0001);
					gateStatus    = GATE_OPEN;
					crossingState = stateSix;
				}
				break;
			case stateSix:
				if (sensorData == 0)
				{
					crossingState = stateOne;
				}
				else if(sensorData & NEAR_1)
				{
					//Close Gates
					PutFIFOData(CROSSING_OUT_ID, 0x0002);
					gateStatus    = GATE_CLOSED;
					crossingState = stateThree;
				}
				else if(sensorData & NEAR_2)
				{
					//Close Gates
					PutFIFOData(CROSSING_OUT_ID, 0x0002);
					gateStatus    = GATE_CLOSED;
					crossingState = stateTwo;
				}
				break;
			default:
				crossingState = stateOne;
				break;
			}
		}
	}
}

//*****************************************************************************
// SelectCrossing
//*****************************************************************************
void SelectCrossing(uint8_t xing)
{
	if(xing >= NUM_CROSSINGS)
	{
		crossingSelect = 0;
		return;
	}

	crossingSelect = xing;
}

//*****************************************************************************
// GetCrossing
//*****************************************************************************
uint8_t GetCrossing(void)
{
	return crossingSelect;
}

//*****************************************************************************
// GetGateStatus
//*****************************************************************************
BOOL GetGateStatus(void)
{
	return gateStatus;
}

// EOF
//lines 188
