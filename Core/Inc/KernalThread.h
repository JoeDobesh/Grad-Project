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

#define TASK_NAME_SIZE 32

typedef enum _PCB_STATES_
{
	READY         = 0,
	RUNNING       = 1,
	UNKNOWN_STATE = 2
}PCB_STATES;

typedef struct tskTaskControlBlock
{
//  volatile portSTACK_TYPE *pxTopOfStack;
															/* Points to the location of
                                                             the last item placed on
                                                             the tasks stack.  THIS
                                                             MUST BE THE FIRST MEMBER
                                                             OF THE STRUCT. */

//  xListItem    xGenericListItem;
															/* List item used to place
                                                             the TCB in ready and
                                                             blocked queues. */
//  xListItem    xEventListItem;
															/* List item used to place
                                                             the TCB in event lists.*/
//  unsigned portBASE_TYPE uxPriority;
															/* The priority of the task
                                                             where 0 is the lowest
                                                             priority. */
//  portSTACK_TYPE *pxStack;
															/* Points to the start of
                                                             the stack. */
//  signed char    pcTaskName[ configMAX_TASK_NAME_LEN ];
															/* Descriptive name given
                                                             to the task when created.
                                                             Facilitates debugging
                                                             only. */

  #if ( portSTACK_GROWTH > 0 )
    portSTACK_TYPE *pxEndOfStack;                         /* Used for stack overflow
                                                             checking on architectures
                                                             where the stack grows up
                                                             from low memory. */
  #endif

  #if ( configUSE_MUTEXES == 1 )
    unsigned portBASE_TYPE uxBasePriority;                /* The priority last
                                                             assigned to the task -
                                                             used by the priority
                                                             inheritance mechanism. */
  #endif

} tskTCB;

typedef void (*USER_PROCESS_PTR)(void);
typedef struct __USER_THREAD__
{
	USER_PROCESS_PTR userThreadPtr;
	BOOL used;
}USER_THREAD;

typedef struct _REGISTERS_
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t sp;
	uint32_t lr;
	uint32_t psr;
	uint32_t pc;
}CORE_REGISTERS;
typedef CORE_REGISTERS * CORE_REGS_PTR;

typedef struct _PCB_
{
	volatile uint32_t * pTopOfStack;
	uint32_t 		 	processId;
	char 			 	name[TASK_NAME_SIZE];
	PCB_STATES     	 	state;
	uint32_t 		 	programCounter;
	uint32_t *			startOfStack;
	uint32_t *			endOfStack;
	uint32_t		 	stackSize;
	CORE_REGISTERS	 	registers;
	USER_PROCESS_PTR 	process;
	void *			 	nextPCB_ptr;
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
