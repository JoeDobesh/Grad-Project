/*
 * PowerControlTask.h
 *
 *  Created on: Jun 7, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_USERAPPS_POWERCONTROLTASK_H_
#define INC_USERAPPS_POWERCONTROLTASK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

void PowerControlInit(void);
uint32_t GetPulseWidth(void);
void PowerControlDecrament(void);
void PowerControlIncrament(void);
void PowerControlTask(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_USERAPPS_POWERCONTROLTASK_H_ */

//lines 11
