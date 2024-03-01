/*
 * Mutex.c
 *
 *  Created on: Nov 3, 2023
 *      Author: joe.dobesh
 */

#include "Mutex.h"

typedef struct _MUTEX_
{
	BOOL locked;
	uint32_t muxId;
} MUTEX;

static MUTEX myMutexes[MUTEX_NUMBER_OF_RESOURCES];

//*****************************************************************************
// MutexInit
//*****************************************************************************
void MutexInit(void)
{
	int i;

	for(i = 0; i < MUTEX_NUMBER_OF_RESOURCES; i++)
	{
		myMutexes[i].locked = FALSE;
		myMutexes[i].muxId = 0;
	}
	printf("MutexInit - Passed\n");
}

//*****************************************************************************
// MutexWait
//*****************************************************************************
void MutexWait(RESOURCES resource)
{
	while(myMutexes[resource].locked == TRUE);
	myMutexes[resource].locked = TRUE;
}

//*****************************************************************************
//* MutexWaitTime
//*****************************************************************************
BOOL MutexWaitTime(RESOURCES resource, uint32_t timer_ms)
{
	uint32_t timeout = HAL_GetTick() + timer_ms;

	while(myMutexes[resource].locked == TRUE)
	{
		if(timeout < HAL_GetTick())
		{
			return FALSE;
		}
	}
	myMutexes[resource].locked = TRUE;

	return TRUE;
}

//*****************************************************************************
//* MutexRelease
//*****************************************************************************
void MutexRelease(RESOURCES resource)
{
	myMutexes[resource].locked = FALSE;
}

// EOF
//lines 40
