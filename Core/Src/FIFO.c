/*
 * FIFO.c
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#include <FIFO.h>

#define FIFO_SIZE 16
#define FIFO_COUNT 8

typedef struct _FIFO
{
	int FIFO_ID;
	unsigned int inputCounter;
	unsigned int outputCounter;
	double* FIFO_Ptr;
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
	printf("FIFO_Init - Passed\n");
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
BOOL RegisterFIFOInput(int id)
{
	for(int i = 0; i < FIFO_COUNT; i++)
	{
		if(fifo[i].FIFO_ID == 0)
		{
			fifo[i].FIFO_Ptr = (double*)malloc(FIFO_SIZE);
			if(fifo[i].FIFO_Ptr == NULL)
			{
				return FALSE;
			}
			fifo[i].FIFO_ID = id;
			fifo[i].inputCounter = 0;
			fifo[i].outputCounter = 0;
			return TRUE;
		}
	}

	return FALSE;
}

//*****************************************************************************
// PutFIFOData
//*****************************************************************************
BOOL PutFIFOData(int id, double data)
{
	for(int i = 0; i < FIFO_COUNT; i++)
	{
		if(fifo[i].FIFO_ID == id)
		{
			if(fifo[i].FIFO_Ptr == NULL)
			{
				return FALSE;
			}
			fifo[i].FIFO_Ptr[fifo[i].inputCounter] = data;
			fifo[i].inputCounter++;
			if(fifo[i].inputCounter >= FIFO_SIZE)
			{
				fifo[i].inputCounter = 0;
			}
			return TRUE;
		}
	}

	return FALSE;
}

//*****************************************************************************
// GetFIFOData
//*****************************************************************************
BOOL GetFIFOData(int id, double* data)
{
	for(int i = 0; i < FIFO_COUNT; i++)
	{
		if(fifo[i].FIFO_ID == id)
		{
			if(fifo[i].inputCounter == fifo[i].outputCounter)
			{
				return FALSE;
			}
			if(fifo[i].FIFO_Ptr == NULL)
			{
				return FALSE;
			}
			*data = fifo[i].FIFO_Ptr[fifo[i].outputCounter];
			fifo[i].outputCounter++;
			if(fifo[i].outputCounter >= FIFO_SIZE)
			{
				fifo[i].outputCounter = 0;
			}
			return TRUE;
		}
	}

	return FALSE;
}

//*****************************************************************************
// ReleaseFIFO
//*****************************************************************************
BOOL ReleaseFIFO(int id)
{
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
			return TRUE;
		}
	}

	return FALSE;
}

//*****************************************************************************
// FIFO_Test
//*****************************************************************************
BOOL FIFO_Test(void)
{
	static int localID = 0x5A;
	static double data;

	if(RegisterFIFOInput(localID) == FALSE)
	{
		printf("Not FIFOs Available\n");
		return FALSE;
	}
	if(PutFIFOData(localID, 0x5AA55AA5) == FALSE)
	{
		printf("FIFO Full");
		return FALSE;
	}
	if(GetFIFOData(localID, &data) == FALSE)
	{
		printf("Broken FIFO\n");
		return FALSE;
	}
	if(data != 0x5AA55AA5)
	{
		printf("FIFO Data Corrupted\n");
		return FALSE;
	}
	if(ReleaseFIFO(localID) == FALSE)
	{
		printf("FIFO Release Failure\n");
		return FALSE;
	}
	printf("FIFO Test Passed");

	return TRUE;
}

// EOF

//lines 160
