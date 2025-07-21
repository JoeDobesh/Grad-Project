/*
 * Modbus.h
 *
 *  Created on: Jun 14, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_MODBUS_H_
#define INC_MODBUS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

#define MODBUS_STACK_SIZE   1000

typedef struct _MODBUD_QUEUE_
{
	uint32_t address;
	uint32_t msgType;
	uint32_t size;
	uint32_t data[2];
}MODBUS_QUEUE;

void ModbusInit(void);
void ModbusTask(void);
void ModbusPollSensorTask(void);
BOOL ModebusGetMessage(uint8_t *, uint8_t *);
BOOL ReadHoldingRegisters(uint16_t, uint16_t, uint8_t);
BOOL WriteSingleRegister(uint16_t, uint16_t, uint8_t);
BOOL ModbusReadSensors(uint8_t *);

#ifdef __cplusplus
}
#endif

#endif /* INC_MODBUS_H_ */

//lines 24
