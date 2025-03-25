/*
 * FIFO.h
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#ifndef INC_FIFO_H_
#define INC_FIFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

void FIFO_Init(void);
void ResetFIFO(void);
BOOL RegisterFIFOInput(int id);
BOOL PutFIFOData(int id, double data);
BOOL GetFIFOData(int id, double* data);
BOOL ReleaseFIFO(int id);

#ifdef __cplusplus
}
#endif

#endif /* INC_FIFO_H_ */

// EOF

//lines 8
