/*
 * Mutex.h
 *
 *  Created on: Nov 3, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_MUTEX_H_
#define INC_MUTEX_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

typedef enum _RESOURCES_
{
	MUTEX_DISK = 0,
	MUTEX_MODBUS,
	MUTEX_DEBUG,
	MUTEX_SERVER,
	MUTEX_PRINT,
	MUTEX_NUMBER_OF_RESOURCES,
}RESOURCES;

void MutexInit(void);
BOOL MutexLock(RESOURCES);
BOOL MutexSpinLock(RESOURCES);
BOOL MutexTimeLock(RESOURCES, uint32_t);
BOOL MutexRelease(RESOURCES);

#ifdef __cplusplus
}
#endif

#endif /* INC_MUTEX_H_ */

//lines 12
