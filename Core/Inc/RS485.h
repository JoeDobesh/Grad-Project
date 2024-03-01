/*
 * RS485.h
 *
 *  Created on: Aug 17, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_RS485_H_
#define INC_RS485_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

void RS485Init(void);
BOOL RS485SendString(uint8_t *,  size_t);
BOOL RS485GetString(uint8_t *, size_t);
void RS485TxInterrupt();
void RS485RxInterrupt(uint16_t);
void RS485Test(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_RS485_H_ */

//lines 8
