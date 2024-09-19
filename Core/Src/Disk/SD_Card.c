/*
 * SD_Card.c
 *
 *  Created on: Aug 21, 2023
 *      Author: joe.dobesh
 */

#include "Disk\SD_Card.h"
#include "Disk\SPI.h"
#include "SoftTimers.h"
#include "Disk/Boot.h"

#define NODATA    0
#define MOREDATA !NODATA

#define cmdGO_IDLE_STATE		0x00 //(00)
#define cmdSEND_OP_COND			0x29 //(41)
#define cmdSEND_IF_COND			0x08 //(08)
#define cmdSEND_CSD				0x49
#define cmdSEND_CID				0x4A
#define cmdSTOP_TRANSMISSION	0x4C
#define cmdSEND_STATUS			0x4D
#define cmdSET_BLOCKLEN			0x50
#define cmdREAD_SINGLE_BLOCK	0x11 //(17)
#define cmdREAD_MULTI_BLOCK		0x52
#define cmdWRITE_SINGLE_BLOCK	0x58
#define cmdWRITE_MULTI_BLOCK	0x59
#define cmdTAG_SECTOR_START		0x60
#define cmdTAG_SECTOR_END		0x61
#define cmdUNTAG_SECTOR			0x62
#define cmdTAG_ERASE_GRP_START	0x63
#define cmdTAG_ERASE_GRP_END	0x64
#define cmdUNTAG_ERASE_GRP		0x65
#define cmdERASE				0x66
#define cmdLOCK_UNLOCK			0x69
#define cmdSD_APP_OP_COND		0x71
#define cmdAPP_CMD				0x37 //(55)
#define cmdREAD_OCR				0x3A //(58)
#define cmdCRC_ON_OFF			0x7B

#define CSD_SIZE         16
#define DATA_START_TOKEN 0xFE

#define SDC_FLOATING_BUS 	0xFF
#define DATA_ACCEPTED		0x05
#define SDC_BAD_RESPONSE	SDC_FLOATING_BUS

#define SYSTEM_CLOCK		((uint32_t)20000000)
#define CLKSPERINSTRUCTION	((uint8_t)4)
#define TMR1PRESCALER		((uint8_t)8)
#define TMR1OVERHEAD		((uint8_t)5)
#define MILLISECDELAY		((uint16_t)((SYSTEM_CLOCK/CLKSPERINSTRUCTION/TMR1PRESCALER/(UINT16_t)1000) - TMR1OVERHEAD)

#define R1_IN_IDLE_STATE		0x01
#define R1_ERASE_RESET			0x02
#define R1_ILLEGAL_COMMAND		0x04
#define R1_COM_CRC_ERROR		0x08
#define R1_ERASE_SEQUENCE_ERROR	0x10
#define R1_ADDRESS_ERROR		0x20
#define R1_PARAMETER_ERROR		0x40

#define R7_COMMAND_VERSION		0xF0
#define R7_VOLTAGE_ACCEPTED		0x0F
#define R7_VOLTAGE_3_3			0x01

typedef union _SDC_STATE_
{
	struct
	{
		uint8_t isSDMMC: 1;
	};
	uint8_t _byte;
}SDC_STATE;

typedef enum _RESPONSE_TYPES_
{
	R1  = 0,
	R1b = 1,
	R2  = 2,
	R3  = 3,
	R7	= 4
} RESPONSE_TYPES;

static const uint8_t sdCommandTable[24][4] =
{
	//Command Name				CRC		Response Type	More Data?
	{ cmdGO_IDLE_STATE,			0x94,	R1,				NODATA},
	{ cmdSEND_OP_COND,			0x00,	R1,				NODATA},
	{ cmdSEND_IF_COND,			0x86,	R7,				NODATA},
	{ cmdSEND_CSD,				0xAF,	R1,				MOREDATA},
	{ cmdSEND_CID,				0x1B,	R1,				MOREDATA},
	{ cmdSTOP_TRANSMISSION,		0xC3,	R1,				NODATA},
	{ cmdSEND_STATUS,			0xAF,	R2,				NODATA},
	{ cmdSET_BLOCKLEN,			0xFF,	R1,				NODATA},
	{ cmdREAD_SINGLE_BLOCK,		0x00,	R1,				MOREDATA},
	{ cmdREAD_MULTI_BLOCK,		0xFF,	R1,				MOREDATA},
	{ cmdWRITE_SINGLE_BLOCK,	0xFF,	R1,				MOREDATA},
	{ cmdWRITE_MULTI_BLOCK,		0xFF,	R1,				MOREDATA},
	{ cmdTAG_SECTOR_START,		0xFF,	R1,				NODATA},
	{ cmdTAG_SECTOR_END,		0xFF,	R1,				NODATA},
	{ cmdUNTAG_SECTOR,			0xFF,	R1,				NODATA},
	{ cmdTAG_ERASE_GRP_START,	0xFF,	R1,				NODATA},
	{ cmdTAG_ERASE_GRP_END,		0xFF,	R1,				NODATA},
	{ cmdUNTAG_ERASE_GRP,		0xFF,	R1,				NODATA},
	{ cmdERASE,					0xDF,	R1b,			NODATA},
	{ cmdLOCK_UNLOCK,			0x89,	R1b,			NODATA},
	{ cmdSD_APP_OP_COND,		0xE5,	R1,				NODATA},
	{ cmdAPP_CMD,				0x00,	R1,				NODATA},
	{ cmdREAD_OCR,				0x00,	R3,				NODATA},
	{ cmdCRC_ON_OFF,			0x25,	R1,				NODATA}
};

typedef enum _COM_TABLE_INDEX_
{
	COM_CODE,
	COM_CRC,
	COM_RESP_TYPE,
	COM_NO_DATA
}COM_TABLE_INDEX;

typedef union _COMMAND_PACKET_
{
	struct
	{
		uint8_t cmd;
		uint8_t addr3; //MSB
		uint8_t addr2;
		uint8_t addr1; //LSB
		uint8_t addr0;
		uint8_t crc;
	};
	uint8_t commandBuffer[6];
}COMMAND_PACKET;
static COMMAND_PACKET commandPacket;

typedef union _OCR_
{
	struct
	{
		uint8_t reg0;
		uint8_t reg1;
		uint8_t reg2;
		uint8_t reg3;
	};
	uint32_t ocrReg;
}OCR;
static OCR gblOCRReg;

struct dataError
{
	uint8_t unknownError:1;
	uint8_t cardConrolError:1;
	uint8_t corectionCodeError:1;
	uint8_t ArgOutOfRange:1;
	uint8_t cardLocked:1;
	uint8_t unused:3;
};

static uint8_t delayTimerHandle = 0;
static BOOT_STATES bootState = NO_CARD;
static BOOL highCapacityCard = TRUE;

//*****************************************************************************
// Send8ClockCycles
//*****************************************************************************
static BOOL Send8ClockCycles(void)
{
	uint8_t buffer[1];

	buffer[0] = 0xFF;
	return SPI_Write(buffer, 1);
}

//*****************************************************************************
// Delayms
//*****************************************************************************
static void Delayms(uint8_t milliseconds)
{
	if ( StartTimer(delayTimerHandle, milliseconds) == FALSE )
	{
		return;
	}
	while(CheckTimer(delayTimerHandle) == FALSE){;}
}

//*****************************************************************************
// ReadCRC
//*****************************************************************************
static void ReadCRC(void)
{
	uint8_t data = 0xFF;

	SPI_Write(&data, 1);
	SPI_Write(&data, 1);
}

//*****************************************************************************
// SD_CardInit
//*****************************************************************************
SDC_ERRORS SD_CardInit(void)
{
	SDC_RESPONSE response;
	SDC_ERRORS status = sdcVALID;
	uint16_t timeout;
	TIMER_PARAMS params;
	uint8_t attempts = 0;

	params.callbackFunctionPtr = NULL;
	params.countTime_ms = 100;
	params.timerType = ONE_SHOT;
	bootState = NO_CARD;
	if( delayTimerHandle == 0 )
	{
		delayTimerHandle = RegisterTimer(params);
		if(delayTimerHandle == 0)
		{
			status = sdcCARD_NOT_INIT_FAILURE;
			return status;
		}
	}
	SPI_Init();
	SPI_CS_High();
	Delayms(100);
	timeout = 0;
	while ( timeout < 50 )
	{
		if ( Send8ClockCycles() == TRUE )
		{
			timeout++;
		}
	}
	response = SD_SendCommand(GO_IDLE_STATE, 0);
	if(response.r1.respByte == SDC_BAD_RESPONSE)
	{
		status = sdcCARD_NOT_INIT_FAILURE;
		printf("SD Card Init - Card not found\n");
		bootState = NO_CARD;
		goto InitError;
	}
	bootState = INIT_FAILED;
	if(!(response.r1.respByte & R1_IN_IDLE_STATE))
	{
		status = sdcCARD_NOT_INIT_FAILURE;
		printf("SD Card Init - Idle State Failure\n");
		goto InitError;
	}
	response = SD_SendCommand(SEND_IF_COND, 0x000001AA);
	if(response.r7.respByte0 & R1_ILLEGAL_COMMAND)
	{
		printf("SD Card Init - Card Version: 1.x\n");
		highCapacityCard = FALSE;
	}
	else
	{
		printf("SD Card Init - Card Version: +2.0\n");
		highCapacityCard = TRUE;
		if ( (response.r7.respByte0 & R1_IN_IDLE_STATE) == 0x00 )
		{
			status = sdcCARD_NOT_INIT_FAILURE;
			printf("SD Card Init - Card no longer in idle state\n");
			goto InitError;
		}
		if(!(response.r7.respByte3 & R7_VOLTAGE_3_3))
		{
			status = sdcCARD_NOT_INIT_FAILURE;
			printf("SD Card Init - Card voltage failure\n");
			goto InitError;
		}
		if(response.r7.respByte4 != 0xAA)
		{
			status = sdcCARD_NOT_INIT_FAILURE;
			printf("SD Card Init - Echo Pattern mismatch\n");
			goto InitError;
		}
	}
	while(1)
	{
		if ( attempts > 100 )
		{
			status = sdcCARD_NOT_INIT_FAILURE;
			printf("SD Card Init - OP_COND Failed\n");
			goto InitError;
		}
		Delayms(10);
		attempts++;
		response = SD_SendCommand(APP_CMD, 0);
		if ( response.r1.respByte > 0x01 )
		{
			continue;
		}
		response = SD_SendCommand(SEND_OP_COND, 0x40000000);
		if ( response.r1.respByte == 0x00 )
		{
			break;
		}
	}
	response = SD_SendCommand(READ_OCR, 0);
	if (response.r3.respByte0 > 0x01)
	{
		status = sdcCARD_NOT_INIT_FAILURE;
		printf("SD Card Init - Could not read the OCR register\n");
		goto InitError;
	}
	else
	{
		gblOCRReg.reg3 = response.r3.respByte1;
		gblOCRReg.reg2 = response.r3.respByte2;
		gblOCRReg.reg1 = response.r3.respByte3;
		gblOCRReg.reg0 = response.r3.respByte4;
		if ( gblOCRReg.reg3 & 0x80 )
		{
			printf("SD_CardInit - Card is ready\n");
		}
		else
		{
			status = sdcCARD_NOT_INIT_FAILURE;
			printf("SD_CardInit - Card is not ready\n");
			goto InitError;
		}
	}
	bootState = CARD_INIT;
	if ( GetMasterBootRecord() == TRUE )
	{
		bootState = BOOT_RECORD;
	}
	else
	{
		status = sdcCARD_NOT_INIT_FAILURE;
		printf("SD_CardInit - Failed to retrieve master boot record.\n");
	}

InitError:

	return(status);
}

//*****************************************************************************
// SD_SendCommand
//*****************************************************************************
SDC_RESPONSE SD_SendCommand(SD_COMMANDS cmd, uint32_t addr)
{
	uint8_t index;
	SDC_RESPONSE response;
	uint16_t timeout = 9;

	commandPacket.cmd = (sdCommandTable[cmd][COM_CODE] | 0x40);
	commandPacket.addr3 = (uint8_t)((addr >> 24) & 0xFF);
	commandPacket.addr2 = (uint8_t)((addr >> 16) & 0xFF);
	commandPacket.addr1 = (uint8_t)((addr >> 8) & 0xFF);
	commandPacket.addr0 = (uint8_t)(addr & 0xFF);
	commandPacket.crc = (sdCommandTable[cmd][COM_CRC] | 0x01);
	Send8ClockCycles();
	SPI_CS_Low();
	Send8ClockCycles();
	if  ( SPI_Write(commandPacket.commandBuffer, sizeof(commandPacket)) == FALSE )
	{
		response.r1.respByte = SDC_BAD_RESPONSE;
		return response;
	}
	if(sdCommandTable[cmd][COM_RESP_TYPE] == R1)
	{
		timeout = 16;
		do
		{
			SPI_Read( &response.r1.respByte, 1 );
			timeout--;
		}while(( response.r1.respByte == 0xFF ) && ( timeout > 0 ));
	}
	else if ( sdCommandTable[cmd][COM_RESP_TYPE] == R1b)
	{
		do
		{
			SPI_Read(&response.r1.respByte, 1);
			timeout--;
		}while((response.r1.respByte == 0xFF)&&(timeout > 0));
		// The R1b response byte has been read
		// Wait for not busy status by reading from the card until a byte doesn't equal 00
		// or a timeout
		//TODO: I don't like this part. He is overwriting response codes. Unused for now.
		response.r1.respByte = 0x00;
		for ( index = 0; index < 0xFF && response.r1.respByte == 0x00; index++ )
		{
			do
			{
				SPI_Read(&response.r1.respByte, 1);
				timeout--;
			}while((response.r1.respByte == 0x00) && (timeout > 0));
		}
	}
	else if ( sdCommandTable[cmd][COM_RESP_TYPE] == R2)
	{
		do
		{
			SPI_Read(&response.r2.respByte0, 1);
			timeout--;
		}while((response.r2.respByte0 == 0xFF) && (timeout > 0));
		if( response.r2.respByte0 != 0xFF )
		{
			SPI_Read(&response.r2.respByte1, 1);
		}
	}
	else if ( sdCommandTable[cmd][COM_RESP_TYPE] == R3 )
	{
		do
		{
			SPI_Read(&response.r3.respByte0, 1);
			timeout--;
		}while((response.r3.respByte0 == 0xFF) && (timeout > 0));
		if( response.r3.respByte0 != 0xFF && (!(response.r3.respByte0 & R1_ILLEGAL_COMMAND)))
		{
			SPI_Read(&response.r3.respByte1, 1);
			SPI_Read(&response.r3.respByte2, 1);
			SPI_Read(&response.r3.respByte3, 1);
			SPI_Read(&response.r3.respByte4, 1);
		}
	}
	else if ( sdCommandTable[cmd][COM_RESP_TYPE] == R7 )
	{
		do
		{
			SPI_Read(&response.r7.respByte0, 1);
			timeout--;
		}while((response.r7.respByte0 == 0xFF) && (timeout > 0));
		if( response.r7.respByte0 != 0xFF && (!(response.r7.respByte0 & R1_ILLEGAL_COMMAND)))
		{
			SPI_Read(&response.r7.respByte1, 1);
			SPI_Read(&response.r7.respByte2, 1);
			SPI_Read(&response.r7.respByte3, 1);
			SPI_Read(&response.r7.respByte4, 1);
		}
	}
	if(sdCommandTable[cmd][COM_NO_DATA] == NODATA)
	{
		Send8ClockCycles();
		SPI_CS_High();
		Send8ClockCycles();
	}

	return response;
}

//*****************************************************************************
// SectorRead
//*****************************************************************************
SDC_ERRORS SectorRead(uint32_t sector_num, uint8_t * buffer)
{
	uint8_t data_token;
	uint16_t index;
	SDC_RESPONSE response;
	SDC_ERRORS status = sdcVALID;

	response = SD_SendCommand(READ_SINGLE_BLOCK, sector_num );
	if ( response.r1.respByte != SDC_FLOATING_BUS )
	{
		index = 0x61B;
		do
		{
			SPI_WriteRead(&data_token, 1);
			index--;
		}while((data_token == SDC_FLOATING_BUS) && (index != 0));
		if ((index == 0) || (data_token != DATA_START_TOKEN))
		{
			status = sdcCARD_TIMEOUT;
			printf("SectorRead - Timeout\n");
		}
		else
		{
			for(index = 0; index < SDC_SECTOR_SIZE; index++)
			{
				SPI_WriteRead(&buffer[index], 1);
			}
			ReadCRC();
		}
	}
	else
	{
		status = sdcCARD_BAD_CMD;
		printf("SectorRead - Failed\n");
	}
	Send8ClockCycles();
	SPI_CS_High();
	Send8ClockCycles();

	return (status);
}

// EOF

//lines 434
