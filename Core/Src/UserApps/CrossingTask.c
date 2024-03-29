/*
 * CrossingTask.c
 *
 *  Created on: Feb 18, 2023
 *      Author: jdobesh
 */

#include "KernalThread.h"
#include "UserApps/CrossingTask.h"
#include "Modbus.h"
#include "SoftTimers.h"

#define NUM_CROSSINGS 3
#define FAR_1	0x0001
#define NEAR_2	0x0002
#define NEAR_1	0x0004
#define FAR_2	0x0008

uint32_t crossingStack[CROSSING_STACK_SIZE];

typedef struct _CROSSING_INFO_
{
	const char* name;
	uint8_t address;
} CROSSING_INFO;

const char cr5[] = {"County Road 5"};
const char dms[] = {"Denham Main Street"};
const char sh2[] = {"State Highway 2"};

static CROSSING_INFO crossings[] =
{
	{cr5, IO_MODULE_0_ADDRESS},
	{dms, IO_MODULE_1_ADDRESS},
	{sh2, IO_MODULE_2_ADDRESS}
};

static STATES crossingState;
static STATES msgState;
static BOOL closeEastSensor;
static BOOL closeWestSensor;
static BOOL farEastSensor;
static BOOL farWestSensor;
static BOOL gateStatus;
static uint32_t timerId;
static uint8_t crossingSelect = 0;

//*****************************************************************************
// CrossingInit
//*****************************************************************************
void CrossingInit(void)
{
	TIMER_PARAMS timerParams;

	timerParams.callbackFunctionPtr = NULL;
	timerParams.countTime_ms = SCAN_TIMEOUT;
	timerParams.timerType = ONE_SHOT;
	timerId = RegisterTimer(timerParams);
	if(timerId == 0)
	{
		printf("Crossing Init - Failed: RegisterTimer");
		return;
	}
	crossingSelect  = 0;
	crossingState   = stateZero;
	msgState        = stateZero;
	closeEastSensor = FALSE;
	closeWestSensor = FALSE;
	farEastSensor   = FALSE;
	farWestSensor   = FALSE;
	gateStatus      = FALSE;
	if(KernalRegister(CrossingTask) == FALSE)
	{
		ReleaseTimer(timerId);
		printf("Crossing Init - Kernel Register Failed\r\n");
		return;
	}
	printf("Crossing Init - Passed\n");
}

//*****************************************************************************
// GetSersorValue
//*****************************************************************************
BOOL GetSersorValue(uint8_t ii)
{
	switch (ii)
	{
	case 0:
		return closeEastSensor;
		break;
	case 1:
		return closeWestSensor;
		break;
	case 2:
		return farEastSensor;
		break;
	case 3:
		return farWestSensor;
		break;
	case 4:
	default:
		return gateStatus;
		break;
	}
}

//*****************************************************************************
// GetSersorValues
//*****************************************************************************
uint8_t GetSersorValues(void)
{
	uint8_t retVal = 0;

	if(closeEastSensor)
	{
		retVal |= 0x02;
	}
	if(closeWestSensor)
	{
		retVal |= 0x04;
	}
	if(farEastSensor)
	{
		retVal |= 0x01;
	}
	if(farWestSensor)
	{
		retVal |= 0x08;
	}

	return retVal;
}
//*****************************************************************************
// AllSensorsClear
//*****************************************************************************
//static BOOL AllSensorsClear(void)
//{
//	if(closeEastSensor == FALSE &&
//	   closeWestSensor == FALSE &&
//	   farEastSensor == FALSE &&
//	   farWestSensor == FALSE)
//	{
//		return TRUE;
//	}

//	return FALSE;
//}

//*****************************************************************************
// Convert2Sixteen
//*****************************************************************************
//static uint16_t Convert2Sixteen(uint8_t * array)
//{
//	uint16_t retVal;

//	retVal = (((uint16_t)array[0]) << 8) & 0xFF00;
//	retVal |= (((uint16_t)array[1]) & 0x00FF);

//	return retVal;
//}

//*****************************************************************************
// GetSensorStatus
//*****************************************************************************
/*
static void GetSensorStatus(uint8_t * data, uint8_t size)
{
	uint8_t count;
	uint16_t retVal;

	count = size;
	if(count <= 1)
	{
		return;
	}
	retVal = Convert2Sixteen(&data[0]);
	if(retVal > 0x00FF)
	{
		closeEastSensor = TRUE;
	}
	count -= 2;
	if(count <= 1)
	{
		return;
	}
	retVal = Convert2Sixteen(&data[2]);
	if(retVal > 0x00FF)
	{
		closeWestSensor = TRUE;
	}
	count -= 2;
	if(count <= 1)
	{
		return;
	}
	retVal = Convert2Sixteen(&data[4]);
	if(retVal > 0x00FF)
	{
		farEastSensor = TRUE;
	}
	count -= 2;
	if(count <= 1)
	{
		return;
	}
	retVal = Convert2Sixteen(&data[6]);
	if(retVal > 0x00FF)
	{
		farWestSensor = FALSE;
	}
}
*/

//*****************************************************************************
// CrossingTask
//*****************************************************************************
void CrossingTask(void)
{
	//uint8_t sensorData[32];
	uint16_t sensorData;
	//uint8_t size;

	/*
	switch(msgState)
	{
	case stateZero:
		if(ReadHoldingRegisters(0x00, 4, crossings[crossingSelect].address) == TRUE)
		{
			msgState++;
			StartTimer(timerId, SCAN_TIMEOUT);
		}
		break;
	case stateOne:
		if (CheckTimer(timerId) == TRUE)
		{
			msgState--;
		}
		if(ModebusGetMessage(sensorData, &size) == TRUE)
		{
			GetSensorStatus(sensorData, size);
			StartTimer(timerId, SCAN_TIMEOUT);
			msgState++;
		}
		break;
	case stateTwo:
	default:
		if (CheckTimer(timerId) == TRUE)
		{
			msgState = stateZero;
		}
		break;
	}
	*/
	sensorData = GetCrossingSensors();
	switch(crossingState)
	{
	case stateZero:
	case stateOne:
			if(sensorData & FAR_1)
			{
				//Close Gates
				WriteSingleRegister(0x00, 1, crossings[crossingSelect].address);
				crossingState = stateTwo;
			}
			else if(sensorData & FAR_2)
			{
				//Close Gates
				WriteSingleRegister(0x00, 1, crossings[crossingSelect].address);
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
				WriteSingleRegister(0x00, 0, crossings[crossingSelect].address);
				crossingState = stateSix;
			}
			break;
		case stateFive:
			if((sensorData & NEAR_1) == 0)
			{
				//Open Gates
				WriteSingleRegister(0x00, 0, crossings[crossingSelect].address);
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
				WriteSingleRegister(0x00, 1, crossings[crossingSelect].address);
				crossingState = stateThree;
			}
			else if(sensorData & NEAR_2)
			{
				//Close Gates
				WriteSingleRegister(0x00, 1, crossings[crossingSelect].address);
				crossingState = stateTwo;
			}
			break;
		default:
			crossingState = stateOne;
			break;
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
