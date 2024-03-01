/*
 * SoftTimers.h
 *
 *  Created on: Feb 15, 2023
 *      Author: jdobesh
 */

#ifndef INC_SOFTTIMERS_H_
#define INC_SOFTTIMERS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

typedef enum _TIMER_TYPE
{
	ONE_SHOT = 0,
	CONTINUOUS
}TIMER_TYPE;

typedef struct _TIMER_PARAMS
{
	TIMER_TYPE timerType;
	uint64_t countTime_ms;
	void (*callbackFunctionPtr)(void);
}TIMER_PARAMS;

typedef struct _TIMER
{
	TIMER_PARAMS params;
	uint64_t counter;
	BOOL registered;
	BOOL active;
}TIMER;

/// <summary>
/// Soft Timers Constructor - Initializes timer and key arrays
/// </summary>
void SoftTimerInit(void);

/// <summary>
///
/// </summary>
void TimerParametersInit(TIMER_PARAMS* parameters);

/// <summary>
/// Reserves a timer and returns the key
/// </summary>
/// <returns>Returns the timer key in the form of a unsigned integer</returns>
uint8_t RegisterTimer(TIMER_PARAMS parameters);

/// <summary>
/// Sets the count down value
/// </summary>
/// <param name='key'>The key of the desired timer in the form of a unsigned integer.</param>
/// <param name='time'>The desired count down time in milliseconds in the form of a unsigned long long.</param>
/// <returns>BOOL - Returns TRUE if the key is valid, otherwise FALSE</returns>
BOOL StartTimer(uint8_t, uint64_t);

/// <summary>
/// Checks if the timer has expired
/// </summary>
/// <param name='key'>The key of the desired timer in the form of a unsigned integer.</param>
/// <returns>BOOL - returns TRUE if the timer has expired, otherwise FALSE</returns>
BOOL CheckTimer(uint32_t);

/// <summary>
/// Frees up the timer
/// </summary>
/// <param name='key'>The key of the desired timer in the form of a unsigned integer.</param>
/// <returns>BOOL - Returns TRUE if the key is valid, otherwise FALSE</returns>
BOOL ReleaseTimer(uint32_t);

/// <summary>
/// Frees up the timer
/// </summary>
void TimerTask(void);

/// <summary>
/// Frees up the timer
/// </summary>
void TimerInterrupt(void);

void StartTimerTest(void);

BOOL TimerTestComplete(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_SOFTTIMERS_H_ */

// EOF
//lines 30
