#ifndef __HEARTBEAT_TASK_H
#define __HEARTBEAT_TASK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

#define HEARTBEAT_STACK_SIZE   4095
#define HEARTBEAT_STACK_SIZE_2 4095

void HeartbeatTask(void);
void HeartbeatTask2(void);

#ifdef __cplusplus
}
#endif

#endif

// EOF

//lines 2
