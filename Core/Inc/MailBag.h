/*
 * MailBag.h
 *
 *  Created on: Feb 15, 2023
 *      Author: jdobesh
 */

#ifndef INC_MAILBAG_H_
#define INC_MAILBAG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"

#define MAX_MESSAGE_SIZE 256

typedef struct _MESSAGE
{
	uint32_t size;
	uint32_t sender;
	uint32_t receiver;
	char* messagePtr;
}MESSAGE;

void MailBagInit(void);
uint32_t RegisterMailBox(uint32_t myAddress);
BOOL ReleaseMailBox(int addr);
BOOL SendMessage(MESSAGE myMessage);
BOOL RetrieveMessage(MESSAGE *myMessage);
void MailbagTask(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_MAILBAG_H_ */

//lines 15
