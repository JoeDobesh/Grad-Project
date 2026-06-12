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

/// <summary>
/// FS_TYPES - enum that defines the different file system values
/// </summary>
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

/// <summary>
/// BootInit - Initializes local static variables
/// </summary>
void BootInit(void);

/// <summary>
/// GetMaterBootRecord - Reads the Master Boot Record, Checks for errors, and determines the number of partitions.
/// </summary>
/// <returns>BOOL - If an error is found, FALSE is returned. Otherwise TRUE</returns>
BOOL GetMasterBootRecord(void);

/// <summary>
/// GetFirstSectorLBA - Reads the Master Boot Record, Checks for errors, and determines the number of partitions.
/// </summary>
/// <returns>BOOL - If an error is found, FALSE is returned. Otherwise TRUE</returns>
uint32_t GetFirstSectorLBA(uint8_t);

/// <summary>
/// GetTotalSectors - Reads the Master Boot Record, Checks for errors, and determines the number of partitions.
/// </summary>
/// <returns>BOOL - If an error is found, FALSE is returned. Otherwise TRUE</returns>
uint32_t GetTotalSectors(uint8_t);

/// <summary>
/// IsBootValid - Reads the Master Boot Record, Checks for errors, and determines the number of partitions.
/// </summary>
/// <returns>BOOL - If an error is found, FALSE is returned. Otherwise TRUE</returns>
BOOL IsBootValid(void);

/// <summary>
/// PrintBootSector - Reads the Master Boot Record, Checks for errors, and determines the number of partitions.
/// </summary>
/// <returns>BOOL - If an error is found, FALSE is returned. Otherwise TRUE</returns>
BOOL PrintBootSector(void);

/// <summary>
/// GetPartitionType - Reads the Master Boot Record, Checks for errors, and determines the number of partitions.
/// </summary>
/// <returns>BOOL - If an error is found, FALSE is returned. Otherwise TRUE</returns>
FS_TYPES GetPartitionType(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_DISK_BOOT_H_ */
