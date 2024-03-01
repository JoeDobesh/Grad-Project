/*
 * SD_Card.h
 *
 *  Created on: Aug 21, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_DISK_SD_CARD_H_
#define INC_DISK_SD_CARD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

#define SDC_SECTOR_SIZE		512

typedef enum _SD_COMMANDS_
{
	GO_IDLE_STATE,
	SEND_OP_COND,
	SEND_IF_COND,
	SEND_CSD,
	SEND_CID,
	STOP_TRANSMISSION,
	SEND_STATUS,
	SET_BLOCKLEN,
	READ_SINGLE_BLOCK,
	READ_MULTI_BLOCK,
	WRITE_SINGLE_BLOCK,
	WRITE_MULTI_BLOCK,
	TAG_SECTOR_START,
	TAG_SECTOR_END,
	UNTAG_SECTOR,
	TAG_ERASE_GRP_START,
	TAG_ERASE_GRP_END,
	UNTAG_ERASE_GRP,
	ERASE,
	LOCK_UNLOCK,
	SD_APP_OP_COND,
	APP_CMD,
	READ_OCR,
	CRC_ON_OFF
}SD_COMMANDS;

typedef union _RESPONSE_1_
{
	uint8_t respByte;
}RESPONSE_1;

typedef union _RESPONSE_2_
{
	struct
	{
		uint8_t respByte0;
		uint8_t respByte1;
	};
}RESPONSE_2;

typedef union _RESPONSE_3_
{
	struct
	{
		uint8_t respByte0;
		uint8_t respByte1;
		uint8_t respByte2;
		uint8_t respByte3;
		uint8_t respByte4;
	};
}RESPONSE_3;

typedef union _RESPONSE_7_
{
	struct
	{
		uint8_t respByte0;
		uint8_t respByte1;
		uint8_t respByte2;
		uint8_t respByte3;
		uint8_t respByte4;
	};
}RESPONSE_7;

typedef union _SDC_RESPONSE_
{
	RESPONSE_1 r1;
	RESPONSE_2 r2;
	RESPONSE_3 r3;
	RESPONSE_7 r7;
}SDC_RESPONSE;

typedef enum _SDC_ERRORS_
{
	sdcVALID = 0,					//No error
	sdcCARD_INIT_COMM_FAILURE,		//Communication hasn't been established with the card
	sdcCARD_NOT_INIT_FAILURE,		//Card did not initialize
	sdcCARD_INIT_TIMEOUT,			//Card initialization timed out
	sdcCARD_TYPE_INVALID,			//Card type was not able to be defined
	sdcCARD_BAD_CMD,				//Card did not recognize the command
	sdcCARD_TIMEOUT,				//Card timed out during a read, write or erase sequence
	sdcCARD_CRC_ERROR,				//A CRC error occurred during a read
	sdcCARD_DATA_REJECTED,			//CRC did not match
	sdcCARD_ERASE_TIMED_OUT			//Erase timed out
}SDC_ERRORS;

typedef enum _BOOT_STATES_
{
	NO_CARD,
	INIT_FAILED,
	CARD_INIT,
	BOOT_RECORD
}BOOT_STATES;

SDC_ERRORS SD_CardInit(void);
SDC_RESPONSE SD_SendCommand(SD_COMMANDS, uint32_t);
SDC_ERRORS SectorRead(uint32_t, uint8_t *);

#ifdef __cplusplus
}
#endif

#endif /* INC_DISK_SD_CARD_H_ */

//lines 93
