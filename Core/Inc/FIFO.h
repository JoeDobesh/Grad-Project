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

#define CROSSING_IN_ID   1
#define CROSSOVER_IN_ID  2
#define CROSSING_OUT_ID  3
#define CROSSOVER_OUT_ID 4

void FIFO_Init(void);
void ResetFIFO(void);
BOOL RegisterFIFOInput(uint8_t id);
BOOL PutFIFOData(uint8_t id, uint16_t data);
BOOL GetFIFOData(uint8_t id, uint16_t* data);
BOOL ReleaseFIFO(uint8_t id);

#ifdef __cplusplus
}
#endif

#endif /* INC_FIFO_H_ */

// EOF

//lines 8
