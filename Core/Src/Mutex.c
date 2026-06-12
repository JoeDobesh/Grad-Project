/*
 * Mutex.c
 *
 *  Created on: Nov 3, 2023
 *      Author: joe.dobesh
 */

#include "Mutex.h"
#include "KernalThread.h"

#define LOCKED   TRUE
#define UNLOCKED FALSE

static BOOL myMutexes[MUTEX_NUMBER_OF_RESOURCES];

//*****************************************************************************
// MutexInit
//*****************************************************************************
void MutexInit(void)
{
	int i;

	for(i = 0; i < MUTEX_NUMBER_OF_RESOURCES; i++)
	{
		myMutexes[i] = UNLOCKED;
	}
}

//*****************************************************************************
// MutexLock
//*****************************************************************************
BOOL MutexLock(RESOURCES resource)
{
	if(resource >= MUTEX_NUMBER_OF_RESOURCES)
	{
		return FALSE;
	}
	DisableAllInterrupts();
	if(myMutexes[resource] == LOCKED)
	{
		EnableAllInterrupts();
		return FALSE;
	}
	myMutexes[resource] = LOCKED;
	EnableAllInterrupts();
	return TRUE;
}

//*****************************************************************************
// MutexSpinLock
//*****************************************************************************
BOOL MutexSpinLock(RESOURCES resource)
{
	if(resource >= MUTEX_NUMBER_OF_RESOURCES)
	{
		return FALSE;
	}
	while(MutexLock(resource) == FALSE){;}

	return TRUE;
}

//*****************************************************************************
//* MutexTimeLock
//*****************************************************************************
BOOL MutexTimeLock(RESOURCES resource, uint32_t timer_ms)
{
	if(resource >= MUTEX_NUMBER_OF_RESOURCES)
	{
		return FALSE;
	}
	uint32_t timeout = HAL_GetTick() + timer_ms;
	while(MutexLock(resource) == FALSE)
	{
		if(timeout < HAL_GetTick())
		{
			return FALSE;
		}
	}

	return TRUE;
}

//*****************************************************************************
//* MutexRelease
//*****************************************************************************
BOOL MutexRelease(RESOURCES resource)
{
	if(resource >= MUTEX_NUMBER_OF_RESOURCES)
	{
		return FALSE;
	}
	DisableAllInterrupts();
	myMutexes[resource] = UNLOCKED;
	EnableAllInterrupts();

	return TRUE;
}

// EOF
