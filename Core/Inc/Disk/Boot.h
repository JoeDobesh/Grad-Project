/*
 * Boot.h
 *
 *  Created on: Sep 15, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_DISK_BOOT_H_
#define INC_DISK_BOOT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"
#include "SD_Card.h"

typedef enum _FS_TYPES_
{
	UNUSED   = 0x00,
	FAT12    = 0x01,
	FAT16    = 0x04,
	EXT      = 0x05,
	FAT16e   = 0x06,
	FAT32    = 0x0B,
	FAT32lba = 0x0C,
	FAT16lba = 0x0E,
	EXTlba   = 0x0F
}FS_TYPES;

void BootInit(void);
BOOL GetMasterBootRecord(void);
uint32_t GetFirstSectorLBA(uint8_t);
uint32_t GetTotalSectors(uint8_t);
BOOL IsBootValid(void);
BOOL PrintBootSector(void);
FS_TYPES GetPartitionType(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_DISK_BOOT_H_ */

//Lines 21
