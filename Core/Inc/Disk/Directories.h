/*
 * Directories.h
 *
 *  Created on: Sep 19, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_DISK_DIRECTORIES_H_
#define INC_DISK_DIRECTORIES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"
#include "FATSystem.h"

#define ATTR_LONG_NAME			0x0F
#define DIR_DEL					0xE5
#define DIR_EMPTY				0
#define DIR_NAMESIZE			8
#define DIR_EXTENSION			3

typedef struct __DIR_ENTRY_
{
	char 	 DIR_Name[DIR_NAMESIZE];			//name
	char	 DIR_Extension[DIR_EXTENSION];	//extension
	uint8_t	 DIR_Attr;						//attributes
	uint8_t  DIR_NTRes;						//reserved
	uint8_t  DIR_CrtTimeTenth;				//time created, tenths
	uint16_t DIR_CrtTime;					//time created
	uint16_t DIR_CrtDate;					//date created
	uint16_t DIR_LstAccDate;				//last access date
	uint16_t DIR_FstClusHI;					//high word of entry's first cluster number
	uint16_t DIR_WrtTime;					//last update time
	uint16_t DIR_WrtDate;					//last update date
	uint16_t DIR_FstClusLO;					//low word of entry's first cluster number
	uint32_t DIR_FileSize;					//file size
} DIR_ENTRY;

typedef DIR_ENTRY * DIR_ENTRY_PTR;

DIR_ENTRY_PTR CacheFileEntry(FILEOBJ, uint16_t *, uint8_t);
BOOL ListRootFiles(DISK *);

#ifdef __cplusplus
}
#endif

#endif /* INC_DISK_DIRECTORIES_H_ */

//lines 26
