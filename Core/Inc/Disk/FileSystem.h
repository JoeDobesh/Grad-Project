/*
 * FileSystem.h
 *
 *  Created on: Sep 19, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_DISK_FILESYSTEM_H_
#define INC_DISK_FILESYSTEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"
#include "SD_Card.h"
#include "FATSystem.h"
#include "Directories.h"

CETYPE FileRead(FILEOBJ, uint8_t *, uint32_t);
CETYPE FileOpen(FILEOBJ, uint16_t *, char);
CETYPE FileClose(FILEOBJ);
CETYPE Filefind(FILEOBJ, FILEOBJ, uint8_t);

#ifdef __cplusplus
}
#endif

#endif /* INC_DISK_FILESYSTEM_H_ */

//lines 9
