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

void ModbusInit(void);
void ModbusTask(void);
void ModbusPollTask(void);
BOOL ModebusGetMessage(uint8_t *, uint8_t *);
BOOL ReadCoils(uint16_t, uint16_t, uint8_t);
BOOL ReadDescreteInputs(uint16_t, uint16_t, uint8_t);
BOOL ReadHoldingRegisters(uint16_t, uint16_t, uint8_t);
BOOL ReadInputRegisters(uint16_t, uint16_t, uint8_t);
BOOL WriteSingleCoil(uint16_t, uint16_t, uint8_t);
BOOL WriteSingleRegister(uint16_t, uint16_t, uint8_t);
BOOL ReadExceptionStatus(uint8_t);
BOOL Diagnostic(uint16_t, uint16_t, uint8_t);
BOOL GetComEventCounter(uint8_t);
BOOL GetComEventLog(uint8_t);
BOOL WriteMultipleCoils(uint16_t, uint16_t, uint8_t, uint8_t);
BOOL WriteMultipleRegisters(uint16_t, uint16_t, uint8_t, uint8_t);
BOOL ReportSlaveID(uint8_t);
BOOL ReadFileRecord(uint8_t, uint8_t);
BOOL WriteFileRecord(uint8_t);
BOOL MaskWriteRegister(uint16_t, uint16_t, uint16_t, uint8_t);
BOOL ReadWriteMultipleRegisters(uint8_t);
BOOL ReadFifoQueue(uint16_t, uint8_t);

void ModbusTest(void);
uint16_t GetCrossingSensors(void);
uint16_t GetCrossoverSensors(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_MODBUS_H_ */

//lines 24
