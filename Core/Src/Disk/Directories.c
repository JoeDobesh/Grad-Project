/*
 * Directories.c
 *
 *  Created on: Sep 19, 2023
 *      Author: joe.dobesh
 */

#include "Directories.h"

#define DIR_ENTRY_SIZE				32
#define DIR_ENTRIES_PER_SECTOR()	(SDC_SECTOR_SIZE/DIR_ENTRY_SIZE)
#define ATTRIB_OFFSET				0x0B
#define ATTRIB_LONG_FILE_NAME		0x0F
#define END_OF_FILE_LIST			0x00
#define UNUSED_ENTRY				0xE5
#define ATTRIB_DIRECTORY			0x10
#define ATTRIB_VOLUME				0x08

extern UART_HandleTypeDef huart3;

//*****************************************************************************
// CacheFileEntry
//*****************************************************************************
DIR_ENTRY_PTR CacheFileEntry(FILEOBJ fo, uint16_t * curEntry, uint8_t forceRead)
{
	DIR_ENTRY_PTR	dir;
	DISK *			disk;
	uint32_t		currentCluster;
	uint32_t		cluster;
	uint32_t		numOfClusters;
	uint16_t		sectorOffset;
	uint16_t		sector;

	disk = fo->disk;
	cluster = fo->dirclus;
	currentCluster = fo->dirCurrentCluster;
	//Get the number of the entry's sector within the directory.
	//A sector can hold 16 directory entries. Shift right 4 times to get the entry number
	//For example, if curEntry < 0x10, it's the directory's first sector and offset2 = 0.
	//If curEntry >= 0x10 and < 0x20, it's the directory's second sector and offset2 = 1.
	sectorOffset = ((*curEntry) >> 4);
	if ( (disk->type == FAT16 && cluster != 0) || (disk->type == FAT32lba && cluster != disk->rootCluster) )
	{
		//It's not the root directory
		//To get the number of the entry's sector within it's cluster,
		//divide the sector number obtained above by the number of sectors per cluster.
		//The remainder (sectorOffset) is the sector's number within it's cluster
		//(The first sector is sector 0)
		sectorOffset = sectorOffset % (disk->SecPerClus);
	}
	if ( forceRead || (*curEntry & 0x0F) == 0)
	{
		//forceRead is TRUE OR the entry is the first one in a sector ((*curEntry & 0x0F) == 0),
		//If either is TRUE,
		//don't assume the current cluster is the cluster to begin looking in for the entry read
		//Instead, read the entry's cluster number from the FAT:

		//Condition 1 - ForceRead
		//Condition 2 - The entry IS in a cluster's first sector (sectorOffset = 0)
		//AND the entry ISN'T in the directory's first cluster
		if (( sectorOffset == 0 && (*curEntry) > DIR_ENTRIES_PER_SECTOR()) || forceRead )
		{
			if(disk->type == FAT16 && cluster == 0)
			{
				//It's the root directory. The current cluster is 0.
				currentCluster = 0;
			}
			else if(disk->type == FAT32lba && cluster == disk->rootCluster)
			{
				currentCluster = disk->rootCluster;
			}
			else
			{
				//it's not the root directory.
				if(forceRead)
				{
					//Get the number of curEntry's cluster within its directory
					// (curEntry / directory entries per cluster)
					// directory entries per cluster = ((directory entries / sector) * (sectors / cluster))
					numOfClusters = (((uint16_t)(*curEntry))/(uint16_t)(((uint16_t)DIR_ENTRIES_PER_SECTOR()) * (uint16_t)disk->SecPerClus));
				}
				else
				{
					//The entry is in a cluster's first sector
					//AND the entry's cluster isn't the first one in the directory
					//AND it's not the root directory
					//Get the next cluster
					numOfClusters = 1;
				}
				//To find the cluster containing curEntry,
				//get the directory's cluster numbers from the FAT until reaching the
				//cluster specified by numofclus or the directory's last cluster
				//On entering the loop, currentCluster = the passed dsk->dirclus member
				while(numOfClusters)
				{
					//Read the next cluster number from the current cluster's FAT entry
					currentCluster = ReadFAT(disk, currentCluster);
					if ( currentCluster >= LAST_CLUSTER )
					{
						//There is no next cluster
						break;
					}
					else
					{
						numOfClusters--;
					}
				}
			}
		} //End: read a cluster number from the FAT
		//We have a cluster number for the entry, either retrieved from the FAT
		//or obtained from the passed FILE structure.
		//If currentCluster is an EOC marker (LAST_CLUSTER code)
		//the directory doesn't have as many clusters as we thought. We can't get an entry.
		if ( currentCluster < LAST_CLUSTER )
		{
			//The current cluster isn't the last one in the file
			//We need to read a sector from the media
			//Store the cluster number in the FILE structure
			fo->dirCurrentCluster = currentCluster;
			//Get the LBA if the cluster's first sector
			sector = Cluster2Sector(disk, currentCluster);
			//If it's the root directory (cluster 0), be sure that the curEntry's sector isn't
			//at or beyond the start of the volume's data area (if FAT16)
			//(curEntry's sector = the cluster's initial sector (sector) +
			//the number of the sector in the cluster containing curEntry (sectorOffset))
			if((disk->type == FAT16) && (currentCluster == 0) && ((sector + sectorOffset) >= disk->firstDataSector))
			{
				dir = ((DIR_ENTRY_PTR)NULL);
			}
			else
			{
				//The sector is in a valid location
				//(either the root->directory area or the volume's data area).
				//Read the data in the sector containing curEntry.
				//sector =  the cluster's first sector
				//offset2 = the number of the sector within the cluster
				if ( SectorRead(sector + sectorOffset, disk->buffer) != sdcVALID )
				{
					dir = ((DIR_ENTRY_PTR)NULL);
				}
				else
				{
					//The sector read was successful
					//Get the requested entry
					if(forceRead)
					{
						//The directory entry is in the DISK structure's buffer member.
						//((*curEntry)%DIR_ENTRIES_PER_SECTOR) =
						//the number of the entry within the sector
						dir = (DIR_ENTRY_PTR)(((DIR_ENTRY_PTR)disk->buffer) + ((*curEntry) % DIR_ENTRIES_PER_SECTOR()));
					}
					else
					{
						//Force read is false, so the entry is the first one in the DISK
						//structure's buffer member
						//(from the if (forceRead | (*curEntry & 0x0F) == 0) test above)
						dir = (DIR_ENTRY_PTR)disk->buffer;
					}
				}
			}//End read an entry from media
		}//End a valid cluster was found
		else
		{
			//The cluster was not valid
			dir = ((DIR_ENTRY_PTR)NULL);
		}
	}
	else
	{
		//Force read is false AND curEntry isn't the first entry in the sector
		//OK to read the directory entry directly from the passed DISK structure's buffer
		//(No need to read a sector from the storage media)
		//((*curEntry) % DIR_ENTRIES_PER_SECTOR) = the number of the entry within the sector
		dir = (DIR_ENTRY_PTR)((DIR_ENTRY_PTR)disk->buffer) + ((*curEntry) % DIR_ENTRIES_PER_SECTOR());
	}

	return dir;
}

//*****************************************************************************
// ListRootFiles
//*****************************************************************************
BOOL ListRootFiles(DISK * disk)
{
	uint8_t * tempBuffer;
	uint32_t currentCluster;
	uint32_t currentSector;
	SDC_ERRORS status;
	BOOL retVal = TRUE;
	uint8_t temp[SDC_SECTOR_SIZE];
	uint16_t counter, index, sfnIndex;
	char ch;

	currentCluster = disk->rootCluster;
	currentSector = Cluster2Sector(disk, currentCluster);
	tempBuffer = malloc(SDC_SECTOR_SIZE);
	if ( tempBuffer == NULL )
	{
		printf("InitFAT: Memory Allocation Failure\n");
		return FALSE;
	}
	status = SectorRead(currentSector, tempBuffer);
	if ( status != sdcVALID )
	{
		printf("InitFAT: SectorRead Failure\n");
		retVal = FALSE;
		goto LRF_FAILED;
	}
	memcpy(&temp[0], tempBuffer, SDC_SECTOR_SIZE);
	ch = '\n';
	HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	for(counter = 0; counter < DIR_ENTRIES_PER_SECTOR(); counter++)
	{
		index = (counter * DIR_ENTRY_SIZE);
		if(temp[index] == END_OF_FILE_LIST)
		{
			break;
		}
		else if(temp[index] == UNUSED_ENTRY)
		{
			continue;
		}
		if((temp[index + ATTRIB_OFFSET] & ATTRIB_LONG_FILE_NAME) == ATTRIB_LONG_FILE_NAME)
		{
			//Long fileName
		}
		else if(temp[index + ATTRIB_OFFSET] & ATTRIB_DIRECTORY)
		{
			//Directory
		}
		else if(temp[index + ATTRIB_OFFSET] & ATTRIB_VOLUME)
		{
			//Volume
			for(sfnIndex = 0; sfnIndex < (DIR_NAMESIZE + DIR_EXTENSION); sfnIndex++)
			{
				ch = temp[index + sfnIndex];
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
			}
			ch = '\n';
			HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
		}
		else
		{
			//Short File
			for(sfnIndex = 0; sfnIndex < DIR_NAMESIZE; sfnIndex++)
			{
				ch = temp[index + sfnIndex];
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
			}
			ch = '.';
			HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
			for(sfnIndex = DIR_NAMESIZE; sfnIndex < (DIR_NAMESIZE + DIR_EXTENSION); sfnIndex++)
			{
				ch = temp[index + sfnIndex];
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
			}
			ch = '\n';
			HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
		}

	}

LRF_FAILED:

	free(tempBuffer);

	return retVal;
}

// EOF

//lines 173
