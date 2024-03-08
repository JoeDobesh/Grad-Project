/*
 * KernalThread.h
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#ifndef INC_KERNALTHREAD_H_
#define INC_KERNALTHREAD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

void KernalTask(void);
BOOL KernalRegister(void * taskPtr);
BOOL KernalRelease(void * taskPtr);
void ContextSwitchInterrupt(void);
BOOL TimeToContextSwitch(void);
void vTaskSwitchContext(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_KERNALTHREAD_H_ */

//EOF

//lines 5
