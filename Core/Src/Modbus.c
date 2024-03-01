/*
 * Modbus.c
 *
 *  Created on: Jun 14, 2023
 *      Author: joe.dobesh
 */

#include "RS485.h"
#include "Modbus.h"
#include "SoftTimers.h"

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


extern UART_HandleTypeDef huart4;

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
static uint16_t holdingRegister;

//*****************************************************************************
// ModbusInit
//*****************************************************************************
void ModbusInit(void)
{
	TIMER_PARAMS timerParams;

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
	else
	{
		printf("Modbus Init - Passed\n");
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
// GetCrossingSensors
//*****************************************************************************
uint16_t GetCrossingSensors(void)
{
	return (holdingRegister & 0x000F);
}

//*****************************************************************************
// GetCrossoverSensors
//*****************************************************************************
uint16_t GetCrossoverSensors(void)
{
	return (holdingRegister & 0x0070);
}

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

	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	bodySize = sendMsg.txDataSize + HEADER_SIZE;
	memcpy(sendBuff, sendMsg.modBuffer, bodySize);
	sendBuff[bodySize+1] = (uint8_t)((sendMsg.crc >> 8) & 0x00FF);
	sendBuff[bodySize] = (uint8_t)(sendMsg.crc & 0x00FF);
	RS485SendString(sendBuff, (sendMsg.txDataSize + HEADER_SIZE + CRC_SIZE));
	messageReceived = FALSE;

	return TRUE;
}

//*****************************************************************************
// ModbusTask
//*****************************************************************************
void ModbusTask(void)
{
	uint8_t inByte;
	uint8_t * ch = &inByte;
	static EXCEPTION_CODES exception;
	static uint8_t dataCounter;

	switch(writeState)
	{
	case IDLE:
		RS485GetString(ch, 1);
		break;
	case WAIT:
		if(CheckTimer(writeTimerId) == TRUE)
		{
			exception = TimeoutError;
			writeState = EXCEPTION;
		}
		else if(RS485GetString(ch, 1) == TRUE)
		{
			rcvMsg.addr = *ch;
			if(rcvMsg.addr == sendMsg.addr)
			{
				writeState++;
			}
			StartTimer(writeTimerId, RESPONSE_TIMEOUT);
		}
		break;
	case FUNCTION_CODE:
		if (CheckTimer(writeTimerId) == TRUE)
		{
			exception = TimeoutError;
			writeState = EXCEPTION;
		}
		else if(RS485GetString(ch, 1) == TRUE)
		{
			rcvMsg.msgType = *ch;
			if((rcvMsg.msgType & EXCEPTION_MASK) == EXCEPTION_MASK)
			{
				exception = *ch;
				writeState = EXCEPTION;
			}
			else if(rcvMsg.msgType != sendMsg.msgType)
			{
				exception = InvalidFunctionCode;
				writeState = EXCEPTION;
			}
			else if(rcvMsg.msgType >= TOTAL_FUCNTION_CODES)
			{
				exception = InvalidFunctionCode;
				writeState = EXCEPTION;
			}
			else
			{
				if(rxFunctionPtrTable[rcvMsg.msgType]() == TRUE)
				{
					dataCounter = 0;
					writeState = CRC_CRC;
					StartTimer(writeTimerId, RESPONSE_TIMEOUT);
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
		if (CheckTimer(writeTimerId) == TRUE)
		{
			exception = TimeoutError;
			writeState = EXCEPTION;
		}
		else if(RS485GetString(ch, 1) == TRUE)
		{
			rcvMsg.crc |= ((uint16_t)(*ch)) << (dataCounter * 8);
			dataCounter++;
			if(dataCounter > 1)
			{
				messageReceived = TRUE;
				writeState = IDLE;
			}
			else
			{
				StartTimer(writeTimerId, RESPONSE_TIMEOUT);
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
		break;
	default:
		writeState = IDLE;
		break;
	}
}

//*****************************************************************************
// ModbusPollTask
//*****************************************************************************
void ModbusPollTask(void)
{
	uint8_t inByte;
	uint8_t * ch = &inByte;
	static EXCEPTION_CODES exception;
	static uint8_t dataCounter;

	switch(readState)
	{
	case IDLE:
		RS485GetString(ch, 1);
		if(CheckTimer(readTimerId) == TRUE)
		{
			if(ReadHoldingRegisters(0, 7, 0x01) == TRUE)
			{
				if(ModbusSend() == TRUE)
				{
					StartTimer(readTimerId, RESPONSE_TIMEOUT);
					readState++;
				}
				else
				{
					StartTimer(readTimerId, POLL_TIMEOUT);
				}
			}
		}
		break;
	case WAIT:
		if(CheckTimer(readTimerId) == TRUE)
		{
			exception = TimeoutError;
			readState = EXCEPTION;
		}
		else if(RS485GetString(ch, 1) == TRUE)
		{
			rcvMsg.addr = *ch;
			if(rcvMsg.addr == sendMsg.addr)
			{
				readState++;
			}
			StartTimer(readTimerId, RESPONSE_TIMEOUT);
		}
		break;
	case FUNCTION_CODE:
		if(CheckTimer(readTimerId) == TRUE)
		{
			exception = TimeoutError;
			readState = EXCEPTION;
		}
		else if(RS485GetString(ch, 1) == TRUE)
		{
			rcvMsg.msgType = *ch;
			if((rcvMsg.msgType & EXCEPTION_MASK) == EXCEPTION_MASK)
			{
				exception = *ch;
				readState = EXCEPTION;
			}
			else if(rcvMsg.msgType != sendMsg.msgType)
			{
				exception = InvalidFunctionCode;
				readState = EXCEPTION;
			}
			else if(rcvMsg.msgType >= TOTAL_FUCNTION_CODES)
			{
				exception = InvalidFunctionCode;
				readState = EXCEPTION;
			}
			else
			{
				if(rxFunctionPtrTable[rcvMsg.msgType]() == TRUE)
				{
					dataCounter = 0;
					readState = CRC_CRC;
					StartTimer(readTimerId, RESPONSE_TIMEOUT);
				}
				else
				{
					exception = InvalidFunctionCode;
					readState = EXCEPTION;
				}
			}
		}
		break;
	case CRC_CRC:
		if (CheckTimer(readTimerId) == TRUE)
		{
			exception = TimeoutError;
			readState = EXCEPTION;
		}
		else if(RS485GetString(ch, 1) == TRUE)
		{
			rcvMsg.crc |= ((uint16_t)(*ch)) << (dataCounter * 8);
			dataCounter++;
			if(dataCounter > 1)
			{
				readState = IDLE;
				StartTimer(readTimerId, POLL_TIMEOUT);
			}
			else
			{
				StartTimer(readTimerId, RESPONSE_TIMEOUT);
			}
		}
		break;
	case EXCEPTION:
		RS485GetString(ch, 1);
		switch(exception)
		{
		case TimeoutError:
			printf ("Modbus::ModbusPollTask - Timeout Error\r\n");
			break;
		case InvalidFunctionCode:
			printf ("Modbus::ModbusPollTask - Invalid Function Code\r\n");
			break;
		case InvalidDataAddress:
			printf ("Modbus::ModbusPollTask - Invalid Data Address\r\n");
			break;
		case InvalidDataValue:
			printf ("Modbus::ModbusPollTask - Invalid Data Value\r\n");
			break;
		case DeviceFailure:
			printf ("Modbus::ModbusPollTask - Device Failure\r\n");
			break;
		default:
			break;
		}
		readState = IDLE;
		StartTimer(readTimerId, POLL_TIMEOUT);
		break;
	default:
		readState = IDLE;
		StartTimer(readTimerId, POLL_TIMEOUT);
		break;
	}
}
//*****************************************************************************
// ReadCoils - 1
//*****************************************************************************
BOOL ReadCoils(uint16_t start, uint16_t num, uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = READ_COILS;
	sendMsg.data[0]    = (uint8_t)((start >> 8) & 0x00FF);
	sendMsg.data[1]    = (uint8_t)(start & 0x00FF);
	sendMsg.data[2]    = (uint8_t)((num >> 8) & 0x00FF);
	sendMsg.data[3]    = (uint8_t)(num & 0x00FF);;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 6);
	sendMsg.txDataSize = 4;

	return ModbusSend();
}

//*****************************************************************************
// ReadDescreteInputs - 2
//*****************************************************************************
BOOL ReadDescreteInputs(uint16_t start, uint16_t num, uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = READ_DESCRETE_INPUTS;
	sendMsg.data[0]    = (uint8_t)((start >> 8) & 0x00FF);
	sendMsg.data[1]    = (uint8_t)(start & 0x00FF);
	sendMsg.data[2]    = (uint8_t)((num >> 8) & 0x00FF);
	sendMsg.data[3]    = (uint8_t)(num & 0x00FF);;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 6);
	sendMsg.txDataSize = 4;

	return ModbusSend();
}

//*****************************************************************************
// ReadHoldingRegisters - 3
//*****************************************************************************
BOOL ReadHoldingRegisters(uint16_t start, uint16_t num, uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
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
// ReadInputRegisters - 4
//*****************************************************************************
BOOL ReadInputRegisters(uint16_t start, uint16_t num, uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = READ_INPUT_REGISTER;
	sendMsg.data[0]    = (uint8_t)((start >> 8) & 0x00FF);
	sendMsg.data[1]    = (uint8_t)(start & 0x00FF);
	sendMsg.data[2]    = (uint8_t)((num >> 8) & 0x00FF);
	sendMsg.data[3]    = (uint8_t)(num & 0x00FF);;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 6);
	sendMsg.txDataSize = 4;

	return ModbusSend();
}

//*****************************************************************************
// WriteSingleCoil - 5
//*****************************************************************************
BOOL WriteSingleCoil(uint16_t address, uint16_t value, uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = WRITE_SINGLE_COIL;
	sendMsg.data[0]    = (uint8_t)((address >> 8) & 0x00FF);
	sendMsg.data[1]    = (uint8_t)(address & 0x00FF);
	sendMsg.data[2]    = (uint8_t)((value >> 8) & 0x00FF);
	sendMsg.data[3]    = (uint8_t)(value & 0x00FF);;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 6);
	sendMsg.txDataSize = 4;

	return ModbusSend();
}

//*****************************************************************************
// WriteSingleRegister - 6
//*****************************************************************************
BOOL WriteSingleRegister(uint16_t address, uint16_t value, uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
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

//*****************************************************************************
// ReadExceptionStatus - 7
//*****************************************************************************
BOOL ReadExceptionStatus(uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = READ_EXCEPTION_STATUS;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 2);
	sendMsg.txDataSize = 0;

	return ModbusSend();
}

//*****************************************************************************
// Diagnostic - 8
//*****************************************************************************
BOOL Diagnostic(uint16_t sub, uint16_t num, uint8_t addr)
{
	uint8_t ii;

	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = DIAGNOSTIC;
	sendMsg.data[0]    = (uint8_t)((sub >> 8) & 0x00FF);
	sendMsg.data[1]    = (uint8_t)(sub & 0x00FF);
	for(ii = 0; ii < sub; ii++)
	{
		sendMsg.data[ii]    = 0;
	}
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 6);
	sendMsg.txDataSize = sub + 4;

	return ModbusSend();
}

//*****************************************************************************
// GetComEventCounter - 11
//*****************************************************************************
BOOL GetComEventCounter(uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = GET_COM_EVENT_COUNTER;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 2);
	sendMsg.txDataSize = 0;

	return ModbusSend();
}

//*****************************************************************************
// GetComEventLog - 12
//*****************************************************************************
BOOL GetComEventLog(uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = GET_COM_EVENT_LOG;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 2);
	sendMsg.txDataSize = 4;

	return ModbusSend();
}

//*****************************************************************************
// WriteMultipleCoils - 15
//*****************************************************************************
BOOL WriteMultipleCoils(uint16_t address, uint16_t quantity, uint8_t count, uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = WRITE_MULTIPLE_COILS;
	sendMsg.data[0]    = (uint8_t)((address >> 8) & 0x00FF);
	sendMsg.data[1]    = (uint8_t)(address & 0x00FF);
	sendMsg.data[2]    = (uint8_t)((quantity >> 8) & 0x00FF);
	sendMsg.data[3]    = (uint8_t)(quantity & 0x00FF);
	sendMsg.data[4]    = count;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 7);
	sendMsg.txDataSize = 5;

	return ModbusSend();
}

//*****************************************************************************
// WriteMultipleRegisters - 16
//*****************************************************************************
BOOL WriteMultipleRegisters(uint16_t address, uint16_t quantity, uint8_t count, uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = WRITE_MULTIPLE_REGISTERS;
	sendMsg.data[0]    = (uint8_t)((address >> 8) & 0x00FF);
	sendMsg.data[1]    = (uint8_t)(address & 0x00FF);
	sendMsg.data[2]    = (uint8_t)((quantity >> 8) & 0x00FF);
	sendMsg.data[3]    = (uint8_t)(quantity & 0x00FF);
	sendMsg.data[4]    = count;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 7);
	sendMsg.txDataSize = 5;

	return ModbusSend();
}

//*****************************************************************************
// ReportSlaveID - 17
//*****************************************************************************
BOOL ReportSlaveID(uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = REPORT_SLAVE_ID;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 2);
	sendMsg.txDataSize = 0;

	return ModbusSend();
}

//*****************************************************************************
// ReadFileRecord - 20
//*****************************************************************************
BOOL ReadFileRecord(uint8_t count, uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = READ_FILE_RECORD;
	sendMsg.data[0]    = count;
	sendMsg.data[1]    = 0x06;
	//sendMsg.data[2]    = (uint8_t)((num >> 8) & 0x00FF);
	//sendMsg.data[3]    = (uint8_t)(num & 0x00FF);;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 4);
	sendMsg.txDataSize = 2;

	return ModbusSend();
}

//*****************************************************************************
// WriteFileRecord - 21
//*****************************************************************************
BOOL WriteFileRecord(uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = WRITE_FILE_RECORD;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 2);
	sendMsg.txDataSize = 0;

	return ModbusSend();
}

//*****************************************************************************
// MaskWriteRegister - 22
//*****************************************************************************
BOOL MaskWriteRegister(uint16_t address, uint16_t andMask, uint16_t orMask, uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = MASK_WRITE_REGISTER;
	sendMsg.data[0]    = (uint8_t)((address >> 8) & 0x00FF);
	sendMsg.data[1]    = (uint8_t)(address & 0x00FF);
	sendMsg.data[2]    = (uint8_t)((andMask >> 8) & 0x00FF);
	sendMsg.data[3]    = (uint8_t)(andMask & 0x00FF);
	sendMsg.data[4]    = (uint8_t)((orMask >> 8) & 0x00FF);
	sendMsg.data[5]    = (uint8_t)(orMask & 0x00FF);;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 8);
	sendMsg.txDataSize = 6;

	return ModbusSend();
}

//*****************************************************************************
// ReadWriteMultipleRegisters - 23
//*****************************************************************************
BOOL ReadWriteMultipleRegisters(uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = READ_WRITE_MULTIPLE_REGISTERS;
	//sendMsg.data[0]    = (uint8_t)((start >> 8) & 0x00FF);
	//sendMsg.data[1]    = (uint8_t)(start & 0x00FF);
	//sendMsg.data[2]    = (uint8_t)((num >> 8) & 0x00FF);
	//sendMsg.data[3]    = (uint8_t)(num & 0x00FF);;
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 6);
	sendMsg.txDataSize = 4;
	//sendMsg.rxDataSize = 2 + (2 * num);

	return ModbusSend();
}

//*****************************************************************************
// ReadFifoQueue - 24
//*****************************************************************************
BOOL ReadFifoQueue(uint16_t address, uint8_t addr)
{
	//if(state != IDLE)
	//{
	//	return FALSE;
	//}
	sendMsg.addr       = addr;
	sendMsg.msgType    = READ_FIFO_QUEUE;
	sendMsg.data[0]    = (uint8_t)((address >> 8) & 0x00FF);
	sendMsg.data[1]    = (uint8_t)(address & 0x00FF);
	sendMsg.crc        = Calculate16BitCRC(sendMsg.modBuffer, 4);
	sendMsg.txDataSize = 2;

	return ModbusSend();
}

//*****************************************************************************
// RxReadCoils - 1
//*****************************************************************************
BOOL RxReadCoils(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxReadDescreteInputs - 2
//*****************************************************************************
BOOL RxReadDescreteInputs(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxReadHoldingRegisters - 3
//*****************************************************************************
BOOL RxReadHoldingRegisters(void)
{
	uint8_t ch;
	uint8_t byteCount, ii;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}
	byteCount = ch;
	for(ii = 0; ii < byteCount; ii++)
	{
		if(RS485GetString(&ch, 1) == FALSE)
		{
			return FALSE;
		}
		rcvMsg.data[ii] = ch;
		holdingRegister = ch;
	}
	rcvMsg.rxDataSize = byteCount;

	return TRUE;
}

//*****************************************************************************
// RxReadInputRegisters - 4
//*****************************************************************************
BOOL RxReadInputRegisters(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxWriteSingleCoil - 5
//*****************************************************************************
BOOL RxWriteSingleCoil(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

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

//*****************************************************************************
// RxReadExceptionStatus - 7
//*****************************************************************************
BOOL RxReadExceptionStatus(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxDiagnostic - 8
//*****************************************************************************
BOOL RxDiagnostic(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxGetComEventCounter - 11
//*****************************************************************************
BOOL RxGetComEventCounter(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxGetComEventLog
//*****************************************************************************
BOOL RxGetComEventLog(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxWriteMultipleCoils
//*****************************************************************************
BOOL RxWriteMultipleCoils(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxWriteMultipleRegisters
//*****************************************************************************
BOOL RxWriteMultipleRegisters(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxReportSlaveID
//*****************************************************************************
BOOL RxReportSlaveID(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxReadFileRecord
//*****************************************************************************
BOOL RxReadFileRecord(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxWriteFileRecord
//*****************************************************************************
BOOL RxWriteFileRecord(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxMaskWriteRegister
//*****************************************************************************
BOOL RxMaskWriteRegister(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxReadWriteMultipleRegisters
//*****************************************************************************
BOOL RxReadWriteMultipleRegisters(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// RxReadFifoQueue
//*****************************************************************************
BOOL RxReadFifoQueue(void)
{
	uint8_t ch;

	if(RS485GetString(&ch, 1) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// NullFunction
//*****************************************************************************
BOOL NullFunction(void)
{
	printf("NullFunction - If you got here, something is really wrong\n");

	return FALSE;
}

// EOF

//lines 777
