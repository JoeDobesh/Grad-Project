/*
 * Boot.c
 *
 *  Created on: Sep 15, 2023
 *      Author: joe.dobesh
 */

#include "Disk\Boot.h"

#define BOOT_SIGNATURE				0xAA55
#define BOOT_SECTOR_ADDRESS			0x0000
#define PARTITION_TABLE_ADDRESS		0x01BE
#define PARTITION_POINTER_0			0x01BE
#define PARTITION_POINTER_1			0x01CE
#define PARTITION_POINTER_2			0x01DE
#define PARTITION_POINTER_3			0x01EE
#define BOOT_SIGNATURE_ADDRESS		0x01FE
#define PARTITION_ENTRY_SIZE		(PARTITION_POINTER_1 - PARTITION_POINTER_0)
#define NUMBER_OF_PARTITION_ENTRIES	4

typedef union _PARTITION_ENTRY_ {
	struct {
		uint8_t bootIndicator;
		uint8_t chsStartingHeadNumber;
		uint16_t chsStartingSectorNumber :6;
		uint16_t chsStartingCylinderNumber98 :2;
		uint16_t chsStartingCylinderNumber70 :8;
		uint8_t partitionType;
		uint8_t chsEndingHeadNumber;
		uint16_t chsEndingSectorNumber :6;
		uint16_t chsEndingCylinderNumber98 :2;
		uint16_t chsEndingCylinderNumber70 :8;
		uint32_t partitionSectorLBA;
		uint32_t totalSectors;
	};
	uint8_t partitionTableEntry[PARTITION_ENTRY_SIZE];
} PARTITION_ENTRY;

static PARTITION_ENTRY partitionEntries[NUMBER_OF_PARTITION_ENTRIES];
static BOOL bootSuccess = FALSE;
static BOOL bootValid = FALSE;

static FS_TYPES partitionType = UNUSED;

//*****************************************************************************
// BootInit
//*****************************************************************************
void BootInit(void) {
	bootSuccess = FALSE;
	bootValid = FALSE;
}

//*****************************************************************************
// GetMasterBootRecord
//*****************************************************************************
BOOL GetMasterBootRecord(void) {
	uint8_t *tempBuffer;
	SDC_ERRORS status;
	uint16_t signature = 0;
	uint8_t index;
	BOOL retVal = TRUE;

	tempBuffer = malloc(SDC_SECTOR_SIZE);
	if (tempBuffer == NULL) {
		printf("GetBootSector: Memory Allocation Failure\n");
		return FALSE;
	}
	status = SectorRead(BOOT_SECTOR_ADDRESS, tempBuffer);
	if (status != sdcVALID) {
		printf("GetBootSector: SectorRead Failure\n");
		bootSuccess = FALSE;
		retVal = FALSE;
		goto BOOT_ERROR;
	}
	signature = ((uint16_t) tempBuffer[BOOT_SIGNATURE_ADDRESS + 1]) << 8;
	signature |= ((uint16_t) tempBuffer[BOOT_SIGNATURE_ADDRESS]);
	if (signature != BOOT_SIGNATURE) {
		printf("GetBootSector: Signature Failure\n");
		bootSuccess = FALSE;
		retVal = FALSE;
		goto BOOT_ERROR;
	}
	if (tempBuffer[0] == 0xEB && tempBuffer[1] == 0x58
			&& tempBuffer[2] == 0x90) {
		printf("Master Boot Record Does Not Exist. Found Boot Sector. Making Concessions\n");
		//TODO: Notify system that there is only one partition with no MBR sector
		bootSuccess = TRUE;
		goto BOOT_ERROR;
	}
	for (index = 0; index < NUMBER_OF_PARTITION_ENTRIES; index++) {
		memcpy(&partitionEntries[index].partitionTableEntry,
				&tempBuffer[PARTITION_POINTER_0 + (PARTITION_ENTRY_SIZE * index)],
				PARTITION_ENTRY_SIZE);
	}
	partitionType = partitionEntries[0].partitionType;
	if (partitionType != FAT32lba && partitionType != FAT16) {
		printf("GetBootSector: Unsupported file system: %d\n", partitionType);
		bootSuccess = FALSE;
		retVal = FALSE;
		goto BOOT_ERROR;
	}
	bootSuccess = TRUE;
	bootValid = TRUE;

	BOOT_ERROR:

	free(tempBuffer);

	return retVal;
}

//*****************************************************************************
// IsBootValid
//*****************************************************************************
BOOL IsBootValid(void) {
	return bootValid;
}

//*****************************************************************************
// GetFirstSectorLBA
//*****************************************************************************
uint32_t GetFirstSectorLBA(uint8_t partitionNumber) {
	if (bootValid == FALSE) {
		return 0;
	}

	return partitionEntries[partitionNumber].partitionSectorLBA;
}

//*****************************************************************************
// GetTotalSectors
//*****************************************************************************
uint32_t GetTotalSectors(uint8_t partitionNumber) {
	if (bootValid == FALSE) {
		return 0;
	}

	return partitionEntries[partitionNumber].totalSectors;
}

//*****************************************************************************
// GetPartitionType
//*****************************************************************************
FS_TYPES GetPartitionType(void) {
	return partitionType;
}

//*****************************************************************************
// PrintBootSector
//*****************************************************************************
BOOL PrintBootSector(void) {
	uint8_t tempBuffer[SDC_SECTOR_SIZE];
	SDC_ERRORS retVal;
	uint8_t row, col;
	uint16_t index;
	uint16_t counter;

	for (counter = 0; counter < 32; counter++) {
		retVal = SectorRead((128 + counter), tempBuffer);
		if (retVal != sdcVALID) {
			printf("PrintBootSector: SectorRead failure\n");
			return FALSE;
		}
		index = 0;
		for (row = 0; row < 32; row++) {
			printf("%03d: ", row);
			for (col = 0; col < 16; col++) {
				printf("0x%02X ", tempBuffer[index++]);
			}
			printf("\n");
		}
		printf("\n");
	}

	return TRUE;
}

// EOF

//lines 147
