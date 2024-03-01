/*
 * SwitchPowerTask.h
 *
 *  Created on: Aug 17, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_USERAPPS_SWITCHPOWERTASK_H_
#define INC_USERAPPS_SWITCHPOWERTASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"
#include "LayoutDefinitions.h"

typedef enum _SWITCH_STATES_
{
	SWITCH_OPEN = 0,
	SWITCH_TRANS,
	SWITCH_CLOSED
}SWITCH_STATES;

void SwitchPowerInit(void);
void SwitchPowerTask(void);
void OpenSwitch(void);
void CloseSwitch(void);
SWITCH_STATES GetSwitchStatus(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_USERAPPS_SWITCHPOWERTASK_H_ */

//EOF

//lines 4
