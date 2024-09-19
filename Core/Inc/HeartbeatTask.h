#ifndef __HEARTBEAT_TASK_H
#define __HEARTBEAT_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"
#include "KernalThread.h"

#define HEARTBEAT_STACK_SIZE_1 2048
#define HEARTBEAT_STACK_SIZE_2 2048

extern uint32_t heartbeatStack1[];
extern uint32_t heartbeatStack2[];

extern PCB heartbeatPCB1;
extern PCB heartbeatPCB2;

void HeartbeatTask1(void);
void HeartbeatTask2(void);

#ifdef __cplusplus
}
#endif

#endif

// EOF

//lines 2
