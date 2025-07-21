/*
 * Modbus.c
 *
 *  Created on: Jun 14, 2023
 *      Author: joe.dobesh
 */

#include "RS485.h"
#include "Modbus.h"
#include "SoftTimers.h"
#include "Mutex.h"
#include "FIFO.h"
#include "MailBag.h"

#define ADU_SIZE			256
#define ADDRESS_BASE 		0
#define ADDRESS_SIZE		1
#define COMMAND_BASE 		(ADDRESS_BASE + ADDRESS_SIZE)
#define COMMAND_SIZE		1
#define DATA_BASE    		(COMMAND_BASE + COMMAND_SIZE)
#define CRC_SIZE			2
#define PDU_SIZE			((ADU_SIZE - ADDRESS_SIZE) - CRC_SIZE)
#define MY_ADDRESS			0xAA
#define RESPONSE_TIMEOUT 	500 //ms
#define POLL_TIMEOUT        500 //ms
#define EXCEPTION_MASK		0x80
#define HEADER_SIZE			2

uint32_t modbusStack[MODBUS_STACK_SIZE];

typedef enum MODBUS_STATES_
{
	IDLE = 0,
	WAIT,
	FUNCTION_CODE,
	CRC_CRC,
	EXCEPTION
} MODBUS_STATES;

typedef enum EXCEPTION_CODES_
{
	TimeoutError        = 0,
	InvalidFunctionCode = 1,
	InvalidDataAddress  = 2,
	InvalidDataValue    = 3,
	DeviceFailure       = 4,
	Acknowledge         = 5,
	DeviceBusy          = 6,
	MemoryParityError   = 8,
	UnavailablePath     = 10,
	GatewayFailure      = 11
} EXCEPTION_CODES;

typedef enum _FUNCTION_CODES_
{
	EMPTY_0							= 0x00, //0
	READ_COILS						= 0x01, //1
	READ_DESCRETE_INPUTS			= 0x02, //2
	READ_HOLDING_REGISTERS			= 0x03, //3
	READ_INPUT_REGISTER				= 0x04, //4
	WRITE_SINGLE_COIL				= 0x05, //5
	WRITE_SINGLE_REGISTER  			= 0x06, //6
	READ_EXCEPTION_STATUS			= 0x07, //7
	DIAGNOSTIC						= 0x08, //8
	EMPTY_9							= 0x09, //9
	EMPTY_10						= 0x0A, //10
	GET_COM_EVENT_COUNTER			= 0x0B, //11
	GET_COM_EVENT_LOG				= 0x0C, //12
	EMPTY_13						= 0x0D, //13
	EMPTY_14						= 0x0E, //14
	WRITE_MULTIPLE_COILS			= 0x0F, //15
	WRITE_MULTIPLE_REGISTERS		= 0x10, //16
	REPORT_SLAVE_ID					= 0x11, //17
	EMPTY_18						= 0x12, //18
	EMPTY_19						= 0x13, //19
	READ_FILE_RECORD				= 0x14, //20
	WRITE_FILE_RECORD				= 0x15, //21
	MASK_WRITE_REGISTER				= 0x16, //22
	READ_WRITE_MULTIPLE_REGISTERS	= 0x17, //23
	READ_FIFO_QUEUE					= 0x18, //24
	TOTAL_FUCNTION_CODES			= 0x19  //25
}FUNCTION_CODES;

typedef union _MSG_STRUCT_
{
	struct
	{
		uint8_t addr;
		FUNCTION_CODES msgType;
		uint8_t data[32];
		uint16_t crc;
		uint8_t txDataSize;
		uint8_t rxDataSize;
	};
	uint8_t modBuffer[ADU_SIZE];
} MSG_STRUCT;

typedef BOOL (*RxFunctionPtr)(void);

BOOL RxReadCoils(void);
BOOL RxReadDescreteInputs(void);
BOOL RxReadHoldingRegisters(void);
BOOL RxReadInputRegisters(void);
BOOL RxWriteSingleCoil(void);
BOOL RxWriteSingleRegister(void);
BOOL RxReadExceptionStatus(void);
BOOL RxDiagnostic(void);
BOOL RxGetComEventCounter(void);
BOOL RxGetComEventLog(void);
BOOL RxWriteMultipleCoils(void);
BOOL RxWriteMultipleRegisters(void);
BOOL RxReportSlaveID(void);
BOOL RxReadFileRecord(void);
BOOL RxWriteFileRecord(void);
BOOL RxMaskWriteRegister(void);
BOOL RxReadWriteMultipleRegisters(void);
BOOL RxReadFifoQueue(void);
BOOL NullFunction(void);

static const RxFunctionPtr rxFunctionPtrTable[TOTAL_FUCNTION_CODES] =
{
	NullFunction,
	RxReadCoils,
	RxReadDescreteInputs,
	RxReadHoldingRegisters,
	RxReadInputRegisters,
	RxWriteSingleCoil,
	RxWriteSingleRegister,
	RxReadExceptionStatus,
	RxDiagnostic,
	NullFunction,
	NullFunction,
	RxGetComEventCounter,
	RxGetComEventLog,
	NullFunction,
	NullFunction,
	RxWriteMultipleCoils,
	RxWriteMultipleRegisters,
	RxReportSlaveID,
	NullFunction,
	NullFunction,
	RxReadFileRecord,
	RxWriteFileRecord,
	RxMaskWriteRegister,
	RxReadWriteMultipleRegisters,
	RxReadFifoQueue
};

static MODBUS_STATES writeState;
static MODBUS_STATES readState;
static uint32_t readTimerId;
static uint32_t writeTimerId;
static MSG_STRUCT sendMsg;
static MSG_STRUCT rcvMsg;
static BOOL messageReceived;
static BOOL busBusy;
//*****************************************************************************
// ModbusInit
//*****************************************************************************
void ModbusInit(void)
{
	TIMER_PARAMS timerParams;

	busBusy = FALSE;
	writeState = IDLE;
	readState = 0;
	messageReceived = FALSE;
	timerParams.callbackFunctionPtr = NULL;
	timerParams.countTime_ms = RESPONSE_TIMEOUT;
	timerParams.timerType = ONE_SHOT;
	writeTimerId = RegisterTimer(timerParams);
	if ( writeTimerId == 0 )
	{
		printf("Modbus Init - Failed: Registering Write Timer");
	}
	timerParams.callbackFunctionPtr = NULL;
	timerParams.countTime_ms = POLL_TIMEOUT;
	timerParams.timerType = ONE_SHOT;
	readTimerId = RegisterTimer(timerParams);
	if ( readTimerId == 0 )
	{
		printf("Modbus Init - Failed: Registering Read Timer");
	}
}

//*****************************************************************************
// Calculate16BitCRC
//*****************************************************************************
static uint16_t Calculate16BitCRC(uint8_t * buf, uint8_t len)
{
	uint16_t crc = 0xFFFF;
	uint16_t pos, i;

	if(len > 256)
	{
		return 0;
	}
	for (pos = 0; pos < len; pos++)
	{
		crc ^= (uint16_t)buf[pos];		// XOR byte into least significant byte of crc
		for (i = 8; i != 0; i--)		// Loop over each bit
		{
			if ((crc & 0x0001) != 0)	// If the LSB is set
			{
				crc >>= 1;				// Shift right and XOR 0xA001
				crc ^= 0xA001;			//? 0x4B37
			}
			else						// Else LSB is not set
			{
				crc >>= 1;				// Just shift right
			}
		}
	}
	// Note, this number has low and high bytes swapped, so use it accordingly (or swap bytes)

	return crc;
}

//*****************************************************************************
// ModbusGetCrossingSensors
//*****************************************************************************
//uint16_t ModbusGetCrossingSensors(void)
//{
//	return (holdingRegister & 0x000F);
//}

//*****************************************************************************
// ModbusGetCrossoverSensors
//*****************************************************************************
//uint16_t ModbusGetCrossoverSensors(void)
//{
//	return (holdingRegister & 0x0070);
//}

//*****************************************************************************
// ModebusGetMessage
//*****************************************************************************
BOOL ModebusGetMessage(uint8_t * data, uint8_t * size)
{
	if(messageReceived == FALSE)
	{
		return FALSE;
	}
	data = rcvMsg.data;
	*size = rcvMsg.rxDataSize;

	return TRUE;
}

//*****************************************************************************
// ModbusSend
//*****************************************************************************
static BOOL ModbusSend(void)
{
	uint8_t sendBuff[ADU_SIZE];
	uint8_t bodySize;

	bodySize = sendMsg.txDataSize + HEADER_SIZE;
	memcpy(sendBuff, sendMsg.modBuffer, bodySize);
	sendBuff[bodySize+1] = (uint8_t)((sendMsg.crc >> 8) & 0x00FF);
	sendBuff[bodySize] = (uint8_t)(sendMsg.crc & 0x00FF);
	if(MutexSpinLock(MUTEX_RS485) == TRUE)
	{
		RS485SendString(sendBuff, (sendMsg.txDataSize + HEADER_SIZE + CRC_SIZE));
		MutexRelease(MUTEX_RS485);
	}
	messageReceived = FALSE;

	return TRUE;
}

//*****************************************************************************
// ModbusParseInputs
//*****************************************************************************
static uint16_t ModbusParseInputs(uint8_t * inputs, uint8_t size)
{
	uint16_t temp;
	uint16_t retVal = 0;
	uint8_t index;
	uint8_t counter = 0;

	for(index = 1; index <= size; index += 2)
	{
		temp = inputs[index] << 8;
		temp = temp | inputs[index + 1];
		if(temp > 0)
		{
			retVal = retVal | (0x0001 << counter);
		}
		counter++;
	}

	return retVal;
}

//*****************************************************************************
// ModbusParseResponse
//*****************************************************************************
static BOOL ModbusParseResponse(MSG_STRUCT * sentMsgPtr,  MSG_STRUCT * retMsgPtr)
{
	EXCEPTION_CODES exception;
	uint32_t timeout;
	uint8_t inByte;
	uint8_t * ch = &inByte;
	uint8_t dataCounter;
	uint8_t writeState = WAIT;
	BOOL retVal = TRUE;

	timeout = HAL_GetTick() + RESPONSE_TIMEOUT;
	while(writeState != IDLE)
	{
		switch(writeState)
		{
		case IDLE:
			writeState++;
			break;
		case WAIT:
			if(HAL_GetTick() > timeout)
			{
				exception = TimeoutError;
				writeState = EXCEPTION;
			}
			else if(RS485GetString(ch, 1) == TRUE)
			{
				retMsgPtr->addr = *ch;
				if(retMsgPtr->addr == sentMsgPtr->addr)
				{
					writeState++;
				}
				timeout = HAL_GetTick() + RESPONSE_TIMEOUT;
			}
			break;
		case FUNCTION_CODE:
			if (HAL_GetTick() > timeout)
			{
				exception = TimeoutError;
				writeState = EXCEPTION;
			}
			else if(RS485GetString(ch, 1) == TRUE)
			{
				retMsgPtr->msgType = *ch;
				if((retMsgPtr->msgType & EXCEPTION_MASK) == EXCEPTION_MASK)
				{
					exception = *ch;
					writeState = EXCEPTION;
				}
				else if(retMsgPtr->msgType != sentMsgPtr->msgType)
				{
					exception = InvalidFunctionCode;
					writeState = EXCEPTION;
				}
				else if(retMsgPtr->msgType >= TOTAL_FUCNTION_CODES)
				{
					exception = InvalidFunctionCode;
					writeState = EXCEPTION;
				}
				else
				{
					if(rxFunctionPtrTable[retMsgPtr->msgType]() == TRUE)
					{
						dataCounter = 0;
						writeState = CRC_CRC;
						timeout = HAL_GetTick() + RESPONSE_TIMEOUT;
					}
					else
					{
						exception = InvalidFunctionCode;
						writeState = EXCEPTION;
					}
				}
			}
			break;
		case CRC_CRC:
			if (HAL_GetTick() > timeout)
			{
				exception = TimeoutError;
				writeState = EXCEPTION;
			}
			else if(RS485GetString(ch, 1) == TRUE)
			{
				retMsgPtr->crc |= ((uint16_t)(*ch)) << (dataCounter * 8);
				dataCounter++;
				if(dataCounter > 1)
				{
					writeState = IDLE;
				}
				else
				{
					timeout = HAL_GetTick() + RESPONSE_TIMEOUT;
				}
			}
			break;
		case EXCEPTION:
			RS485GetString(ch, 1);
			switch(exception)
			{
			case TimeoutError:
				break;
			case InvalidFunctionCode:
				break;
			case InvalidDataAddress:
				break;
			case InvalidDataValue:
				break;
			case DeviceFailure:
				break;
			default:
				break;
			}
			writeState = IDLE;
			retVal = FALSE;
			break;
		default:
			writeState = IDLE;
			break;
		}
	}

	return retVal;
}

//*****************************************************************************
// ModbusTask
//*****************************************************************************
void ModbusTask(void)
{
	uint16_t sensorData;
	uint16_t crossingData;
	uint16_t crossoverData;
	uint8_t inByte;
	BOOL error;

	TIMER_PARAMS timerParams;
	uint8_t timerId;

	timerParams.callbackFunctionPtr = NULL;
	timerParams.countTime_ms        = POLL_TIMEOUT;
	timerParams.timerType           = ONE_SHOT;

	if(RegisterFIFOInput(CROSSING_IN_ID) == FALSE)
	{
		printf("Crossing Task Init - Failed: RegisterFIFOInput");
		while(TRUE){;}
	}
	if(RegisterFIFOInput(CROSSOVER_IN_ID) == FALSE)
	{
		printf("Crossing Task Init - Failed: RegisterFIFOInput");
		while(TRUE){;}
	}
	timerId = RegisterTimer(timerParams);
	if(timerId == 0)
	{
		printf("Modbus Init - Failed: RegisterTimer");
		while(TRUE){;}
	}
	if(StartTimer(timerId, timerParams.countTime_ms) == FALSE)
	{
		printf("Modbus Init - Failed: StartTimer");
		while(TRUE){;}
	}
	while(TRUE)
	{
		if(CheckTimer(timerId) == TRUE)
		{
			RS485GetString(&inByte, 1);
			writeState = WAIT;
			if(ReadHoldingRegisters(0x81, 7, 0x01) == TRUE)
			{
				if(ModbusParseResponse(&sendMsg, &rcvMsg) == TRUE)
				{
					sensorData = ModbusParseInputs(rcvMsg.data, rcvMsg.rxDataSize);
					MutexSpinLock(MUTEX_FIFO);
					error = PutFIFOData(CROSSING_IN_ID, sensorData);
					MutexRelease(MUTEX_FIFO);
					if( error == FALSE)
					{
						printf("Modbus Task Init - Failed: PutFIFOData");
					}
					MutexSpinLock(MUTEX_FIFO);
					error = PutFIFOData(CROSSOVER_IN_ID, sensorData);
					MutexRelease(MUTEX_FIFO);
					if(error == FALSE)
					{
						printf("Modbus Task Init - Failed: PutFIFOData");
					}
					messageReceived = TRUE;
				}
			}
		}
		if(GetFIFOData(CROSSING_OUT_ID, &crossingData) == TRUE)
		{
			if(WriteSingleRegister(0x0000, crossingData, 0x01) == TRUE)
			{
				if(ModbusParseResponse(&sendMsg, &rcvMsg) == TRUE)
				{

				}
			}
		}
//		if(GetFIFOData(CROSSOVER_OUT_ID, &crossoverData) == TRUE)
//		{
//			if(WriteSingleRegister(0x0001, crossingData, 0x01) == TRUE)
//			{
//				if(ModbusParseResponse(&sendMsg, &rcvMsg) == TRUE)
//				{
//
//				}
//			}
//		}
	}
}

//*****************************************************************************
// ReadHoldingRegisters - 3
//*****************************************************************************
BOOL ReadHoldingRegisters(uint16_t start, uint16_t num, uint8_t addr)
{
	sendMsg.addr       = addr;
	sendMsg.msgType    = READ_HOLDING_REGISTERS;
	sendMsg.data[0]    = (uint8_t)((start >> 8) & 0x00FF);
	sendMsg.data[1]    = (uint8_t)(start & 0x00FF);
	sendMsg.data[2]    = (uint8_t)((num >> 8) & 0x00FF);
	sendMsg.data[3]    = (uint8_t)(num & 0x00FF);;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 6);
	sendMsg.txDataSize = 4;

	return ModbusSend();
}

//*****************************************************************************
// WriteSingleRegister - 6
//*****************************************************************************
BOOL WriteSingleRegister(uint16_t address, uint16_t value, uint8_t addr)
{
	sendMsg.addr       = addr;
	sendMsg.msgType    = WRITE_SINGLE_REGISTER;
	sendMsg.data[0]    = (uint8_t)((address >> 8) & 0x00FF);
	sendMsg.data[1]    = (uint8_t)(address & 0x00FF);
	sendMsg.data[2]    = (uint8_t)((value >> 8) & 0x00FF);
	sendMsg.data[3]    = (uint8_t)(value & 0x00FF);
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 6);;
	sendMsg.txDataSize = 4;

	return ModbusSend();
}

BOOL NullFunction(void){return TRUE;}
BOOL RxReadCoils(void){return TRUE;}
BOOL RxReadDescreteInputs(void){return TRUE;}
BOOL RxReadInputRegisters(void){return TRUE;}
BOOL RxWriteSingleCoil(void){return TRUE;}
BOOL RxReadExceptionStatus(void){return TRUE;}
BOOL RxDiagnostic(void){return TRUE;}
BOOL RxGetComEventCounter(void){return TRUE;}
BOOL RxGetComEventLog(void){return TRUE;}
BOOL RxWriteMultipleCoils(void){return TRUE;}
BOOL RxWriteMultipleRegisters(void){return TRUE;}
BOOL RxReportSlaveID(void){return TRUE;}
BOOL RxReadFileRecord(void){return TRUE;}
BOOL RxWriteFileRecord(void){return TRUE;}
BOOL RxMaskWriteRegister(void){return TRUE;}
BOOL RxReadWriteMultipleRegisters(void){return TRUE;}
BOOL RxReadFifoQueue(void){return TRUE;}

//*****************************************************************************
// RxReadHoldingRegisters - 3
//*****************************************************************************
BOOL RxReadHoldingRegisters(void)
{
	uint8_t ch;
	uint8_t ii;

	for(ii = 0; ii < 14; ii++)
	{
		if(RS485GetString(&ch, 1) == FALSE)
		{
			return FALSE;
		}
		rcvMsg.data[ii] = ch;
	}
	rcvMsg.rxDataSize = 14;

	return TRUE;
}

//*****************************************************************************
// RxWriteSingleRegister - 6
//*****************************************************************************
BOOL RxWriteSingleRegister(void)
{
	uint8_t ch;
	uint8_t ii;

	for(ii = 0; ii < 4; ii++)
	{
		if(RS485GetString(&ch, 1) == FALSE)
		{
			return FALSE;
		}
		rcvMsg.data[ii] = ch;
	}
	rcvMsg.rxDataSize = 4;

	return TRUE;
}

// EOF

//lines 777
