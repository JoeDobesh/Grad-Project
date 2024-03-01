/*
 * FileSystem.c
 *
 *  Created on: Sep 19, 2023
 *      Author: joe.dobesh
 */

#include "FileSystem.h"

#define ATTR_MASK	0x3F
#define ATTR_HIDDEN	0x02
#define ATTR_VOLUME	0x08
#define DIR_DEL		0xE5
#define DIR_EMPTY	0
#define FOUND		0
#define NOT_FOUND	1
#define NO_MORE		2

#define ATTR_ARCHIVE	0x20
#define DIR_NAMECOMP	(DIR_NAMESIZE + DIR_EXTENSION)

#define RAMread(a, f) *(a + f)

//*****************************************************************************
// FillFileObject
//*****************************************************************************
uint8_t FillFileObject(FILEOBJ fo, uint16_t * fHandle)
{
	DIR_ENTRY_PTR	dir;
	uint16_t		temp;
	uint8_t			a;
	uint8_t			index;
	uint8_t			status;
	uint8_t			test = 0;
	uint8_t			character;

	dir = CacheFileEntry(fo, fHandle, FALSE);
	a = dir->DIR_Name[0];
	if ( dir == (DIR_ENTRY_PTR)NULL || a == DIR_EMPTY )
	{
		status = NO_MORE;
	}
	else
	{
		if ( a == DIR_DEL )
		{
			status = NOT_FOUND;
		}
		else
		{
			status = FOUND;
			for ( index = 0; index < DIR_NAMESIZE; index++ )
			{
				character = dir->DIR_Name[index];
				fo->name[test++] = character;
			}
			character = dir->DIR_Extension[0];
			if ( character != ' ' )
			{
				for ( index = 0; index < DIR_EXTENSION; index++ )
				{
					character = dir->DIR_Extension[index];
					fo->name[test++] = character;
				}
			}
			fo->entry = *fHandle;
			fo->fileSize = (dir->DIR_FileSize);
			temp = (dir->DIR_FstClusHI << 16);
			temp = temp | dir->DIR_FstClusLO;
			fo->firstCluster = temp;
			fo->time = dir->DIR_WrtTime;
			fo->date = dir->DIR_WrtDate;
			a = dir->DIR_Attr;
			fo->attributes = a;
		}//End: Entry isn't deleted
	}//End: an entry exists

	return status;
}

//*****************************************************************************
// FileRead
//*****************************************************************************
CETYPE FileRead(FILEOBJ fo, uint8_t * dest, uint32_t count)
{
	DISK *		disk;
	CETYPE		error = CE_GOOD;
	uint32_t	lba;
	uint32_t	seek;
	uint32_t	size;
	uint32_t	temp;
	uint32_t	pos;

	disk = (DISK*)fo->disk;
	temp = count;
	//Save the offset to begin reading from within the current sector,
	//the offset to read from within the file, and the file's size
	pos  = fo->currentSectorByte;
	seek = fo->currentByte;
	size = fo->fileSize;
	//Get the sector number of the files's current cluster
	lba = Cluster2Sector(disk, fo->currentCluster);
	//Add the number of the current sector within the cluster
	lba = lba + fo->currentSector;
	//Read the sector's data
	if ( SectorRead(lba, disk->buffer) != sdcVALID )
	{
		error = CE_BAD_SECTOR_READ;
	}
	//Read from the file until finished or an error
	while ( error == CE_GOOD && temp > 0 )
	{
		if ( seek == size )
		{
			//It's the end of the file
			error = CE_EOF;
		}
		else
		{
			//If we've reached the end of a sector, load another sector
			if ( pos == SDC_SECTOR_SIZE )
			{
				//Read the offset within the sector
				pos = 0;
				//Increment the sector number
				fo->currentSector++;
				//The sector number (sec) should be a value between 0 and SecPerClus - 1
				//If sec = SecPerclus, the sector is the first sector in a cluster
				if ( fo->currentSector == disk->SecPerClus )
				{
					//Get the next cluster in the file and start in the cluster's first sector
					fo->currentSector = 0;
					error = FILE_GetNextCluster(fo, 1);
				}
				if ( error == CE_GOOD )
				{
					//Get the sector number of the current cluster, which may have changed
					lba = Cluster2Sector(disk, fo->currentCluster);
					//Add the number of the current sector within the cluster
					lba = lba + fo->currentSector;
					//Read the sector's data
					if ( SectorRead(lba, disk->buffer) != sdcVALID )
					{
						error = CE_BAD_SECTOR_READ;
					}
				}
			}//End: load new sector
			if ( error == CE_GOOD )
			{
				//A sector's data is in the DISK structure's buffer member
				//Copy a byte from the specified offset (pos) in the DISK structure's buffer
				//to the dest buffer
				*dest = RAMread(disk->buffer, pos);
				pos++;
				dest = dest + 1; //Increment the dest buffer offset
				seek++;			 //Increment the number of the byte to copy
				temp--;		 //Decrement the number of bytes remaining to copy
			}
		}//End: if not end of file
	}//while no error and more bytes to copy
	//Save the offset within the sector
	fo->currentSectorByte = pos;
	//Save the offset within the file
	fo->currentByte = seek;

	return error;
}

//*****************************************************************************
// FileOpen
//*****************************************************************************
CETYPE FileOpen(FILEOBJ fo, uint16_t * fHandle, char type)
{
	DISK *		disk;
	CETYPE 		error = CE_GOOD;
	uint32_t	lba;
	uint8_t		retVal;

	disk = (DISK *)(fo->disk);
	if ( disk->mount == FALSE )
	{
		error = CE_NOT_INIT;
	}
	else
	{
		//Get the file's directory entry and store the directory's sector
		//in the disk->buffer member of the file structure (fo)
		CacheFileEntry(fo, fHandle, TRUE);
		//Fill the file structure with information from the directory entry
		retVal = FillFileObject(fo, fHandle);
		if ( retVal != FOUND )
		{
			error = CE_FILE_NOT_FOUND;
		}
		else
		{
			fo->currentByte 		= 0;
			fo->currentCluster		= fo->firstCluster;
			fo->currentSector		= 0;
			fo->currentSectorByte	= 0;
			//Determine the LBA of the file's current cluster
			lba = Cluster2Sector(disk, fo->currentCluster);
			//Read the cluster's first sector into the DISK structure's buffer member
			if ( SectorRead(lba, disk->buffer) != sdcVALID)
			{
				error = CE_BAD_SECTOR_READ;
			}
			//Set the FILE structure's flags
			fo->flags.FileWriteEOF = FALSE;
			if ( type == 'w' || type == 'a' )
			{
				//Open the file for writing or appending
				fo->flags.write = 1;
			}
			else
			{
				//Open the file for reading
				fo->flags.write = 0;
			}
		}//End: a file was found
	}//End: the media is available

	return error;
}

//*****************************************************************************
// FileClose
//*****************************************************************************
CETYPE FileClose(FILEOBJ fo)
{
	CETYPE error = CE_GOOD;

	return error;
}

//*****************************************************************************
// FILEfind
//*****************************************************************************
CETYPE Filefind(FILEOBJ foDest, FILEOBJ foCompareTo, uint8_t cmd)
{
	CETYPE		statusB = CE_FILE_NOT_FOUND;
	uint16_t	fHandle = 0;
	uint16_t	attrib;
	uint8_t		index;
	uint8_t		state;
	uint8_t		character;
	uint8_t		test;

	//Set the destination FILE structure's current cluster to the directory's cluster
	foDest->dirCurrentCluster = foDest->dirclus;
	//Read a directory entry
	if ( CacheFileEntry(foDest, &fHandle, TRUE) == NULL )
	{
		statusB = CE_BADCACHEREAD;
	}
	else
	{
		//Read entries until finding the file or the end of the directory
		while(TRUE)
		{
			if ( statusB != CE_GOOD )
			{
				//Store information about the file
				state = FillFileObject(foDest, &fHandle);
				if ( state == NO_MORE )
				{
					//The entry doesn't exist
					break;
				}
			}
			else
			{
				//There was a problem in reading the file information
				break;
			}
			if ( state == FOUND )
			{
				//An entry was found. Read the attributes
				attrib = foDest->attributes;
				attrib &= ATTR_MASK;
				//If the entry is for a volume ID or hidden file, skip it
				if (( attrib != ATTR_VOLUME ) && (attrib & ATTR_HIDDEN) != ATTR_HIDDEN)
				{
					statusB = CE_GOOD;
					character = (uint8_t)'m'; //random value
					//Look for a name match
					for ( index = 0; (statusB == CE_GOOD) && index < DIR_NAMECOMP; index++ )
					{
						//Get a character from the found file's name
						character = foDest->name[index];
						//Get the corresponding character from the file name we're searching for
						test = foCompareTo->name[index];
						if ( tolower(character) != tolower(test) )
						{
							//Quit the loop if a character doesn't match
							statusB = CE_FILE_NOT_FOUND;
						}
					}
				}
			}//End: An entry was found
			else
			{
				//An empty or deleted entry was found
				if ( cmd == 2 )
				{
					statusB = CE_GOOD;
				}
			}
			//Increment the number of the entry in the directory
			fHandle++;
		}//End: loop until found or end of directory
	}//End: Cache_File_Enrty was successful

	return statusB;
}

//EOF

//lines 233
