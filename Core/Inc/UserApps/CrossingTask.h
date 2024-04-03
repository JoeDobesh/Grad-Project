/*
 * CrossingTask.h
 *
 *  Created on: Feb 18, 2023
 *      Author: jdobesh
 */

#ifndef INC_USERAPPS_CROSSINGTASK_H_
#define INC_USERAPPS_CROSSINGTASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"
#include "LayoutDefinitions.h"

#define CROSSING_STACK_SIZE 1000

#define GATE_CLOSED TRUE
#define GATE_OPEN	FALSE

void CrossingTask(void);
uint8_t GetSersorValues(void);
void SelectCrossing(uint8_t);
uint8_t GetCrossing(void);
BOOL GetGateStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_USERAPPS_CROSSINGTASK_H_ */

// EOF

//lines 4
