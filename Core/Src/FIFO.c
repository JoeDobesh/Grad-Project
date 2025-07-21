/*
 * FIFO.c
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#include <FIFO.h>
#include "KernalThread.h"

#define FIFO_SIZE 16
#define FIFO_COUNT 8

typedef struct _FIFO
{
	uint8_t FIFO_ID;
	uint16_t inputCounter;
	uint16_t outputCounter;
	uint16_t * FIFO_Ptr;
}FIFO;

static FIFO fifo[FIFO_COUNT];

//*****************************************************************************
// FIFO_Init
//*****************************************************************************
void FIFO_Init(void)
{
	for(int i = 0; i < FIFO_COUNT; i++)
	{
		fifo[i].FIFO_ID = 0;
		fifo[i].inputCounter = 0;
		fifo[i].outputCounter = 0;
		fifo[i].FIFO_Ptr = NULL;
	}
}

//*****************************************************************************
// ResetFIFOs
//*****************************************************************************
void ResetFIFOs(void)
{
	for(int i = 0; i < FIFO_COUNT; i++)
	{
		if(fifo[i].FIFO_Ptr != NULL)
		{
			free(fifo[i].FIFO_Ptr);
			fifo[i].FIFO_Ptr = NULL;
			fifo[i].inputCounter = 0;
			fifo[i].outputCounter = 0;
			fifo[i].FIFO_ID = 0;
		}
	}
}

//*****************************************************************************
// RegisterFIFOInput
//*****************************************************************************
BOOL RegisterFIFOInput(uint8_t id)
{
	BOOL retVal = FALSE;

	DisableAllInterrupts();
	for(int i = 0; i < FIFO_COUNT; i++)
	{
		if(fifo[i].FIFO_ID == 0)
		{
			fifo[i].FIFO_Ptr = malloc(FIFO_SIZE);
			if(fifo[i].FIFO_Ptr == NULL)
			{
				return retVal;
			}
			fifo[i].FIFO_ID = id;
			fifo[i].inputCounter = 0;
			fifo[i].outputCounter = 0;
			retVal = TRUE;
			break;
		}
	}
	EnableAllInterrupts();

	return retVal;
}

//*****************************************************************************
// PutFIFOData
//*****************************************************************************
BOOL PutFIFOData(uint8_t id, uint16_t data)
{
	BOOL retVal = FALSE;

	DisableAllInterrupts();
	for(int i = 0; i < FIFO_COUNT; i++)
	{
		if(fifo[i].FIFO_ID == id)
		{
			if(fifo[i].FIFO_Ptr == NULL)
			{
				return retVal;
			}
			fifo[i].FIFO_Ptr[fifo[i].inputCounter] = data;
			fifo[i].inputCounter++;
			if(fifo[i].inputCounter >= FIFO_SIZE)
			{
				fifo[i].inputCounter = 0;
			}
			retVal = TRUE;
			break;
		}
	}
	EnableAllInterrupts();

	return retVal;
}

//*****************************************************************************
// GetFIFOData
//*****************************************************************************
BOOL GetFIFOData(uint8_t id, uint16_t* data)
{
	BOOL retVal = FALSE;

	DisableAllInterrupts();
	for(int i = 0; i < FIFO_COUNT; i++)
	{
		if(fifo[i].FIFO_ID == id)
		{
			if(fifo[i].inputCounter == fifo[i].outputCounter)
			{
				return retVal;
			}
			if(fifo[i].FIFO_Ptr == NULL)
			{
				return retVal;
			}
			*data = fifo[i].FIFO_Ptr[fifo[i].outputCounter];
			fifo[i].outputCounter++;
			if(fifo[i].outputCounter >= FIFO_SIZE)
			{
				fifo[i].outputCounter = 0;
			}
			retVal = TRUE;
			break;
		}
	}
	EnableAllInterrupts();

	return retVal;
}

//*****************************************************************************
// ReleaseFIFO
//*****************************************************************************
BOOL ReleaseFIFO(uint8_t id)
{
	BOOL retVal = FALSE;

	DisableAllInterrupts();
	for(int i = 0; i < FIFO_COUNT; i++)
	{
		if(fifo[i].FIFO_ID == id)
		{
			if(fifo[i].FIFO_Ptr != NULL)
			{
				free(fifo[i].FIFO_Ptr);
				fifo[i].FIFO_Ptr = NULL;
			}
			fifo[i].inputCounter = 0;
			fifo[i].outputCounter = 0;
			fifo[i].FIFO_ID = 0;
			retVal = TRUE;
			break;
		}
	}
	EnableAllInterrupts();

	return retVal;
}

// EOF

//lines 160
