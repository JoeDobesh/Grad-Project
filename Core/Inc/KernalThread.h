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

#define HANDLER_MODE_MAIN_EXT		(0xFFFFFFE1)
#define THREAD_MODE_MAIN_EXT		(0xFFFFFFE9)
#define THREAD_MODE_PROCESS_EXT		(0xFFFFFFED)
#define HANDLER_MODE_MSP			(0xFFFFFFF1)
#define THREAD_MODE_MSP				(0xFFFFFFF9)
#define THREAD_MODE_PSP				(0xFFFFFFFD)

extern int32_t interruptDisableCounter;

#define DisableAllInterrupts()		\
	{								\
		interruptDisableCounter++;	\
		__asm volatile("cpsid i");	\
	}

#define EnableAllInterrupts()				\
	{										\
		interruptDisableCounter--;			\
		if(interruptDisableCounter <= 0)	\
		{									\
			interruptDisableCounter = 0;	\
			__asm volatile("cpsie i");		\
		}									\
	}

typedef struct _PCB_
{
	volatile uint32_t * pTopOfStack;
	uint32_t * pStackLimit;
	volatile void     *	nextPCB_ptr;
}PCB;

void KernalTask(void);
BOOL TimeToContextSwitch(void);
void vTaskSwitchContext(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_KERNALTHREAD_H_ */

//EOF

//lines 5
