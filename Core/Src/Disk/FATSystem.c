/*
 * FileSystem.c
 *
 *  Created on: Sep 15, 2023
 *      Author: joe.dobesh
 */

#include "Disk\FATSystem.h"

#define FAIL 0
#define RAMreadW(a, f) *(uint32_t *)(a+f)
#define RAMwrite(a, f, d) *(a + f)=d
#define CLUSTER_EMPTY	0x00000000
#define END_CLUSTER		0xFFFFFFFF

DISK myDisk;

static uint16_t	numOfReservedSectors = 0;

//*****************************************************************************
// InitFAT
//*****************************************************************************
BOOL InitFAT(void)
{
	uint8_t * tempBuffer;
	SDC_ERRORS status;
	BOOL retVal = TRUE;

	if(IsBootValid() == FALSE)
	{
		printf("InitFAT - Boot Invalid\n");
		return FALSE;
	}
	tempBuffer = malloc(SDC_SECTOR_SIZE);
	if ( tempBuffer == NULL )
	{
		printf("InitFAT: Memory Allocation Failure\n");
		return FALSE;
	}
	myDisk.firstSectorNumber = GetFirstSectorLBA(0);
	status = SectorRead(myDisk.firstSectorNumber, tempBuffer);
	if ( status != sdcVALID )
	{
		printf("InitFAT: SectorRead Failure\n");
		retVal = FALSE;
		goto INIT_FAT_FAILED;
	}
	if ((tempBuffer[0] != 0xEB ) && (tempBuffer[1] != 0x58) && (tempBuffer[2] != 0x90))
	{
		printf("InitFAT: Jump Instruction Check Failed\n");
		retVal = FALSE;
		goto INIT_FAT_FAILED;
	}
	myDisk.type = GetPartitionType();
	myDisk.bytesPerSector = ((uint16_t)tempBuffer[0x0C]) << 8;
	myDisk.bytesPerSector |= ((uint16_t)tempBuffer[0x0B]);
	if(myDisk.bytesPerSector != SDC_SECTOR_SIZE)
	{
		printf("InitFAT: Bytes Per Sector do not match\n");
		retVal = FALSE;
		goto INIT_FAT_FAILED;
	}
	myDisk.SecPerClus = tempBuffer[0x0D];
	numOfReservedSectors = (((uint16_t)tempBuffer[0x0F]) << 8);
	numOfReservedSectors |= ((uint16_t)tempBuffer[0x0E]);
	myDisk.firstFAT_Sector = myDisk.firstSectorNumber + numOfReservedSectors;
	myDisk.fatCopies = tempBuffer[0x10];
	myDisk.totalSectors = ((uint32_t)tempBuffer[0x23]) << 24;
	myDisk.totalSectors |= ((uint32_t)tempBuffer[0x22]) << 16;
	myDisk.totalSectors |= ((uint32_t)tempBuffer[0x21]) << 8;
	myDisk.totalSectors |= ((uint32_t)tempBuffer[0x20]);
	if(GetTotalSectors(0) != myDisk.totalSectors)
	{
		printf("InitFAT: Total Sectors does not match\n");
		retVal = FALSE;
		goto INIT_FAT_FAILED;
	}
	if(myDisk.type == FAT32lba)
	{
		myDisk.sectorsPerFAT = ((uint32_t)tempBuffer[0x27]) << 24;
		myDisk.sectorsPerFAT |= ((uint32_t)tempBuffer[0x26]) << 16;
		myDisk.sectorsPerFAT |= ((uint32_t)tempBuffer[0x25]) << 8;
		myDisk.sectorsPerFAT |= ((uint32_t)tempBuffer[0x24]);
	}
	else if(myDisk.type == FAT16)
	{
		myDisk.sectorsPerFAT |= ((uint32_t)tempBuffer[0x17]) << 8;
		myDisk.sectorsPerFAT |= ((uint32_t)tempBuffer[0x16]);
	}
	myDisk.firstDataSector = myDisk.firstFAT_Sector + (myDisk.fatCopies * myDisk.sectorsPerFAT);
	if(myDisk.type == FAT32lba)
	{
		myDisk.rootCluster = ((uint32_t)tempBuffer[0x2F]) << 24;
		myDisk.rootCluster |= ((uint32_t)tempBuffer[0x2E]) << 16;
		myDisk.rootCluster |= ((uint32_t)tempBuffer[0x2D]) << 8;
		myDisk.rootCluster |= ((uint32_t)tempBuffer[0x2C]);
	}
	else if(myDisk.type == FAT16)
	{
		myDisk.rootCluster = myDisk.firstDataSector;
	}
	if(tempBuffer[0x1FE] != 0x55 && tempBuffer[0x1FF] != 0xAA)
	{
		printf("InitFAT: Signature Failure\n");
		retVal = FALSE;
		goto INIT_FAT_FAILED;
	}

INIT_FAT_FAILED:

	free(tempBuffer);

	return retVal;
}

//*****************************************************************************
// ReadFAT
//*****************************************************************************
uint32_t ReadFAT(DISK * disk, uint32_t currentCluster)
{
	uint32_t cluster;
	uint32_t lba;
	uint32_t p;

	//Get the address of the file's current cluster
	//The address is two bytes LSB first
	p = currentCluster;
	//The LBA of the FAT sector containing the cluster's data is the FAT's starting address
	//plus the high byte of the current cluster
	//(Each sector contains 128 four byte entries)
	lba = disk->firstFAT_Sector + (p >> 8);
	//Read the sector
	if( SectorRead(lba, disk->buffer) != sdcVALID)
	{
		return CLUSTER_FAIL;
	}
	//To get the value stored in the cluster's entry
	//read 4 bytes in the buffer of retrieved data
	//beginning at offset = low byte of current cluster's address << 1
	//Multiply by 4 because each entry is 4 bytes
	cluster = RAMreadW(disk->buffer, ((p & 0xFF) * 4));
	if(cluster >= LAST_CLUSTER)
	{
		cluster = LAST_CLUSTER;
	}

	return cluster;
}

//*****************************************************************************
// FILE_GetNextCluster
//*****************************************************************************
uint8_t FILE_GetNextCluster(FILEOBJ fo, uint32_t n)
{
	uint32_t nextCluster;
	uint32_t currentCluster;
	DISK * disk;
	uint8_t error = CE_GOOD;

	disk = fo->disk;
	do {
		currentCluster = fo->currentCluster;
		if (( nextCluster = ReadFAT(disk, currentCluster)) == FAIL)
		{
			error = CE_BAD_SECTOR_READ;
		}
		else
		{
			if ( nextCluster >= disk->maxcls )
			{
				error = CE_INVALID_CLUSTER;
			}
			currentCluster = LAST_CLUSTER;
			if ( nextCluster >= currentCluster )
			{
				error = CE_FAT_EOF;
			}
		}
		fo->currentCluster = nextCluster;
	} while (--n > 0 && error == CE_GOOD);

	return error;
}

//*****************************************************************************
// Cluster2Sector
//*****************************************************************************
uint32_t Cluster2Sector(DISK * disk, uint32_t cluster)
{
	uint32_t sector;

	if(cluster == 0 || cluster == 1)
	{
		sector = disk->rootCluster + cluster; //TODO: I don't think this is right
	}
	else
	{
		sector = ((cluster - 2) * disk->SecPerClus) + disk->firstDataSector;
	}

	return (sector);
}

// EOF
//lines 157
