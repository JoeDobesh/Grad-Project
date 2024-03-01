/*
 * RealTimeClock.h
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#ifndef INC_REALTIMECLOCK_H_
#define INC_REALTIMECLOCK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

void RealTimeClockInit(void);
uint32_t RealTimeClockRead(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_REALTIMECLOCK_H_ */

// EOF

//lines 3
