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
	MUTEX_FIFO,
	MUTEX_MAILBOX,
	MUTEX_RS485,
	MUTEX_DEBUG,
	MUTEX_SERVER,
	MUTEX_PRINT,
	MUTEX_GET_TICK,
	MUTEX_SOFT_TIMER,
	MUTEX_NUMBER_OF_RESOURCES,
}RESOURCES;

/// <summary>
/// MutexInit - Initializes all resource blocks to unlocked
/// </summary>
void MutexInit(void);

/// <summary>
/// MutexLock - Checks if the desired resource is available.
/// The resource is locked if it is available.
/// </summary>
/// <param name='resource'>The desired resource block that is to be acquired.</param>
/// <returns>BOOL - Returns TRUE if the resource was available.
/// Returns FALSE is the resource is unavailable or unknown.</returns>
BOOL MutexLock(RESOURCES resource);

/// <summary>
/// MutexSpinLock - Continuously checks if the desired resource is available.
/// The resource is locked once it becomes available.
/// </summary>
/// <param name='resource'>The desired resource block that is to be acquired.</param>
/// <returns>BOOL - Returns TRUE if the resource is available, returns FALSE if the resource block in unknown</returns>
BOOL MutexSpinLock(RESOURCES resource);

/// <summary>
/// MutexTimeLock - Continuously checks if the desired resource is available for a prescribed time.
/// It acquires it when it becomes available within that time. If the timer expires the function returns.
/// </summary>
/// <param name='resource'>The desired resource block that is to be acquired.</param>
/// <returns>BOOL - Returns TRUE if the resource is available.
/// Returns FALSE if the resource is unknown or the timer expires.</returns>
BOOL MutexTimeLock(RESOURCES resource, uint32_t timer_ms);

/// <summary>
/// MutexRelease - Releases the resource block.
/// </summary>
/// <param name='resource'>The desired resource block that is to be released.</param>
/// <returns>BOOL - Returns FALSE if the resource is unknown, otherwise TRUE.</returns>
BOOL MutexRelease(RESOURCES resource);

#ifdef __cplusplus
}
#endif

#endif /* INC_MUTEX_H_ */
