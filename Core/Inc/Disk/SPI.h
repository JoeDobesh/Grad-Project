/*
 * SPI.h
 *
 *  Created on: Aug 18, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_SPI_H_
#define INC_SPI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

void SPI_Init(void);
void SPI_CS_High(void);
void SPI_CS_Low(void);
BOOL SPI_Read(uint8_t *, uint16_t);
BOOL SPI_Write(uint8_t *, uint16_t);
BOOL SPI_WriteRead(uint8_t *, uint16_t);

#ifdef __cplusplus
}
#endif

#endif /* INC_SPI_H_ */

//lines 7
