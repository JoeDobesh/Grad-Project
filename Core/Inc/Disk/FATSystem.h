/*
 * FileSystem.h
 *
 *  Created on: Sep 15, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_DISK_FATSYSTEM_H_
#define INC_DISK_FATSYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"
#include "Boot.h"

#define AVAILABLE_CLUSTER	0x00000000
#define RESERVED_CLUSTER	0x00000001
#define CLUSTER_FAIL		0xFFFFFFFF
#define LAST_CLUSTER		0xFFFFFFF8
#define FILE_NAME_SIZE 11

typedef enum _CE_TYPE_
{
	CE_GOOD				= 0,
	CE_NOT_INIT			= 6,
	CE_BAD_SECTOR_READ	= 7,
	CE_WRITE_ERROR		= 8,
	CE_INVALID_CLUSTER	= 9,
	CE_FILE_NOT_FOUND	= 10,
	CE_DIR_FULL			= 17,
	CE_DISK_FULL		= 18,
	CE_WRITE_PROTECTED	= 22,
	CE_BADCACHEREAD		= 28,
	CE_FAT_EOF			= 60,
	CE_EOF				= 61
}CETYPE;

typedef struct
{
	uint8_t	write:1;
	uint8_t FileWriteEOF:1;
}FILE_FLAGS;

typedef struct
{
	uint8_t  buffer[SDC_SECTOR_SIZE];
	uint32_t bytesPerSector;
	uint32_t firstSectorNumber;
	uint32_t firstFAT_Sector;
	uint32_t rootCluster;
	uint32_t firstDataSector;
	uint32_t sectorsPerFAT;
	uint32_t totalSectors;
	uint32_t maxcls;
	uint16_t maxroot;
	uint8_t  fatCopies;
	uint8_t  SecPerClus;
	FS_TYPES type;
	BOOL     mount;
}DISK;

typedef struct
{
	DISK *     disk;
	uint32_t   firstCluster;
	uint32_t   currentCluster;
	uint32_t   currentSector;
	uint32_t   currentSectorByte;
	uint32_t   currentByte;
	uint32_t   fileSize;
	FILE_FLAGS flags;
	uint16_t   time;
	uint16_t   date;
	char       name[FILE_NAME_SIZE];
	uint16_t   entry;
	uint16_t   chk;
	uint16_t   attributes;
	uint32_t   dirclus;
	uint32_t   dirCurrentCluster;
}FILE_DAT;

typedef FILE_DAT * FILEOBJ;

extern DISK myDisk;

BOOL InitFAT(void);
uint32_t ReadFAT(DISK *, uint32_t);
uint32_t Cluster2Sector(DISK *, uint32_t);
uint8_t FILE_GetNextCluster(FILEOBJ, uint32_t);

#ifdef __cplusplus
}
#endif

#endif /* INC_DISK_FATSYSTEM_H_ */

//lines 68
