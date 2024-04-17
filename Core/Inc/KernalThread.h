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

void DisableContext(void);
void EnableContext(void);
void KernalTask(void);
BOOL TimeToContextSwitch(void);
void vTaskSwitchContext(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_KERNALTHREAD_H_ */

//EOF

//lines 5
