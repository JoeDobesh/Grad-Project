/*
 * MailBag.c
 *
 *  Created on: Feb 15, 2023
 *      Author: jdobesh
 */

#include "MailBag.h"
#include "Mutex.h"
#include "SoftTimers.h"

#define MAX_MAIL_BOXES 16
#define MAX_MESSAGE_COUNT 16

uint32_t mailbagStack[MAILBAG_STACK_SIZE];

typedef struct _MAIL_BOX
{
	uint32_t address;
	int txHeadCounter;
	int txTailCounter;
	int rxHeadCounter;
	int rxTailCounter;
	MESSAGE inBox[MAX_MESSAGE_COUNT];
	MESSAGE outBox[MAX_MESSAGE_COUNT];
}MAIL_BOX;

static MAIL_BOX mailBoxes[MAX_MAIL_BOXES];

//*****************************************************************************
// MailBagInit
//*****************************************************************************
void MailBagInit(void)
{
	for(int i = 0; i < MAX_MAIL_BOXES; i++)
	{
		mailBoxes[i].address = 0;
		mailBoxes[i].txHeadCounter = 0;
		mailBoxes[i].rxHeadCounter = 0;
		mailBoxes[i].txTailCounter = 0;
		mailBoxes[i].rxTailCounter = 0;
		for(int j = 0; j < MAX_MESSAGE_COUNT; j++)
		{
			mailBoxes[i].inBox[j].messagePtr = NULL;
			mailBoxes[i].outBox[j].messagePtr = NULL;
			mailBoxes[i].inBox[j].size = 0;
			mailBoxes[i].inBox[j].receiver = 0;
			mailBoxes[i].inBox[j].sender = 0;
			mailBoxes[i].outBox[j].size =0;
			mailBoxes[i].outBox[j].receiver = 0;
			mailBoxes[i].outBox[j].sender = 0;
		}
	}
}

//*****************************************************************************
// ResetMailBox
//*****************************************************************************
static void ResetMailBox(uint32_t addr)
{
	for(int j = 0; j < MAX_MESSAGE_COUNT; j++)
	{
		if(mailBoxes[addr].inBox[j].messagePtr != NULL)
		{
			free(mailBoxes[addr].inBox[j].messagePtr);
			mailBoxes[addr].inBox[j].messagePtr = NULL;
			mailBoxes[addr].inBox[j].size = 0;
			mailBoxes[addr].inBox[j].receiver = 0;
			mailBoxes[addr].inBox[j].sender = 0;
		}
		if(mailBoxes[addr].outBox[j].messagePtr != NULL)
		{
			free(mailBoxes[addr].outBox[j].messagePtr);
			mailBoxes[addr].outBox[j].messagePtr = NULL;
			mailBoxes[addr].outBox[j].size =0;
			mailBoxes[addr].outBox[j].receiver = 0;
			mailBoxes[addr].outBox[j].sender = 0;
		}
	}
}

//*****************************************************************************
// RegisterMailBox
//*****************************************************************************
uint32_t RegisterMailBox(uint32_t myAddress)
{
	if(myAddress == 0)
	{
		return 0;
	}
	for(int i = 0; i < MAX_MAIL_BOXES; i++)
	{
		if(mailBoxes[i].address == myAddress)
		{
			return myAddress;
		}
	}
	for(int i = 0; i < MAX_MAIL_BOXES; i++)
	{
		if(mailBoxes[i].address == 0)
		{
			mailBoxes[i].address = myAddress;
			return myAddress;
		}
	}

	return 0;
}

//*****************************************************************************
// ReleaseMailBox
//*****************************************************************************
BOOL ReleaseMailBox(uint32_t addr)
{
	if(addr == 0)
	{
		return FALSE;
	}
	for(int i = 0; i < MAX_MAIL_BOXES; i++)
	{
		if(mailBoxes[i].address == addr)
		{
			ResetMailBox(i);
			mailBoxes[i].address = 0;
			return TRUE;
		}
	}

	return FALSE;
}

//*****************************************************************************
// SendMessage
//*****************************************************************************
BOOL SendMessage(MESSAGE myMessage)
{
	MAIL_BOX *myMailBox;
	uint32_t currentCount, nextCount, endCount;

	if((myMessage.sender == 0) || (myMessage.receiver == 0) ||
	   (myMessage.size > MAX_MESSAGE_SIZE) || (myMessage.messagePtr == NULL))
	{
		return FALSE;
	}
	for(int i = 0; i < MAX_MAIL_BOXES; i++)
	{
		if( mailBoxes[i].address == myMessage.sender)
		{
			myMailBox = &mailBoxes[i];
			currentCount = myMailBox->txHeadCounter;
			endCount = myMailBox->txTailCounter;
			nextCount = ((currentCount + 1) == MAX_MESSAGE_COUNT)? 0: currentCount;
			if(nextCount == endCount)
			{
				return FALSE;
			}
			myMailBox->outBox[currentCount].messagePtr = malloc(myMessage.size);
			if(myMailBox->outBox[currentCount].messagePtr == NULL)
			{
				return FALSE;
			}
			memcpy(myMailBox->outBox[currentCount].messagePtr, myMessage.messagePtr, myMessage.size);
			myMailBox->outBox[currentCount].sender = myMessage.sender;
			myMailBox->outBox[currentCount].receiver = myMessage.receiver;
			myMailBox->outBox[currentCount].size = myMessage.size;
			myMailBox->txHeadCounter = nextCount;
			return TRUE;
		}
	}

	return FALSE;
}

//*****************************************************************************
// RetrieveMessage
//*****************************************************************************
BOOL RetrieveMessage(MESSAGE *myMessage)
{
	if(myMessage == NULL)
	{
		return FALSE;
	}
	if(myMessage->receiver == 0 || myMessage->size < MAX_MESSAGE_SIZE)
	{
		return FALSE;
	}
	for(int i = 0; i < MAX_MAIL_BOXES; i++)
	{
		if(mailBoxes[i].address == myMessage->receiver)
		{
			if(mailBoxes[i].rxTailCounter == mailBoxes[i].rxHeadCounter)
			{
				return FALSE;
			}
			memcpy(myMessage->messagePtr, mailBoxes[i].inBox[mailBoxes[i].rxTailCounter].messagePtr, mailBoxes[i].inBox[mailBoxes[i].rxTailCounter].size);
			myMessage->size = mailBoxes[i].inBox[mailBoxes[i].rxTailCounter].size;
			myMessage->sender = mailBoxes[i].inBox[mailBoxes[i].rxTailCounter].sender;
			free (mailBoxes[i].inBox[mailBoxes[i].rxTailCounter].messagePtr);
			mailBoxes[i].inBox[mailBoxes[i].rxTailCounter].messagePtr = NULL;
			mailBoxes[i].rxTailCounter++;
			mailBoxes[i].rxTailCounter = (mailBoxes[i].rxTailCounter == MAX_MESSAGE_COUNT)? 0: mailBoxes[i].rxTailCounter;
			return TRUE;
		}
	}

	return FALSE;
}

//*****************************************************************************
// MailbagTask
//*****************************************************************************
void MailbagTask(void)
{
	uint32_t tempAddress;
	TIMER_PARAMS mailTimer;
	uint8_t timerID;

	mailTimer.callbackFunctionPtr = NULL;
	mailTimer.timerType = ONE_SHOT;
	mailTimer.countTime_ms = 500;
	timerID = RegisterTimer(mailTimer);

	if (timerID == 0)
	{
		printf("MailbagTask - Failed to create soft timer\n");
	}
	else
	{
		if (StartTimer(timerID, 500) == FALSE)
		{
			printf("MailbagTask - Failed to start soft timer\n");
		}
	}
	while(TRUE)
	{
		if (CheckTimer(timerID) == TRUE)
		{
			for ( int i = 0; i < MAX_MAIL_BOXES; i++)
			{
				MutexSpinLock(MUTEX_MAILBOX);
				if(mailBoxes[i].txHeadCounter == mailBoxes[i].txTailCounter)
				{
					continue;
				}
				tempAddress = mailBoxes[i].outBox[mailBoxes[i].txTailCounter].receiver;
				for ( int j = 0; j < MAX_MAIL_BOXES; j++)
				{
					if ( mailBoxes[j].address == tempAddress)
					{
						mailBoxes[j].inBox[mailBoxes[j].rxHeadCounter].messagePtr = malloc(mailBoxes[i].outBox[mailBoxes[i].txTailCounter].size);
						if(mailBoxes[j].inBox[mailBoxes[j].rxHeadCounter].messagePtr == NULL)
						{
							return;
						}
						memcpy(mailBoxes[j].inBox[mailBoxes[j].rxHeadCounter].messagePtr, mailBoxes[i].outBox[mailBoxes[i].txTailCounter].messagePtr, mailBoxes[i].outBox[mailBoxes[i].txTailCounter].size);
						mailBoxes[j].inBox[mailBoxes[j].rxHeadCounter].receiver = tempAddress;
						mailBoxes[j].inBox[mailBoxes[j].rxHeadCounter].sender = mailBoxes[i].outBox[mailBoxes[i].txTailCounter].sender;
						mailBoxes[j].inBox[mailBoxes[j].rxHeadCounter].size = mailBoxes[i].outBox[mailBoxes[i].txTailCounter].size;
						mailBoxes[j].rxHeadCounter++;
						mailBoxes[j].rxHeadCounter = (mailBoxes[j].rxHeadCounter == MAX_MESSAGE_COUNT)? 0: mailBoxes[j].rxHeadCounter;
						free (mailBoxes[i].outBox[mailBoxes[i].txTailCounter].messagePtr);
						mailBoxes[i].outBox[mailBoxes[i].txTailCounter].messagePtr = NULL;
						mailBoxes[i].txTailCounter++;
						mailBoxes[i].txTailCounter = (mailBoxes[i].txTailCounter == MAX_MESSAGE_COUNT)? 0: mailBoxes[i].txTailCounter;
						break;
					}
				}
				MutexRelease(MUTEX_MAILBOX);
			}
			if (StartTimer(timerID, 500) == FALSE)
			{
				printf("MailbagTask - Failed to start soft timer\n");
			}
		}
	}
}

//*****************************************************************************
// TestMailSystem
//*****************************************************************************
BOOL TestMailSystem(void)
{
	uint8_t testStep = 0;
	uint32_t sender, receiver;
	MESSAGE myMessage;
	char messageStr[] = "The quick brown fox jumped over the lazy dog.\n";
	BOOL retVal;
	uint8_t tries;

	while(TRUE)
	{
		switch(testStep)
		{
		case 0:
			printf("Test Mail System - Creating 2 Mail Boxes\n");
			sender = RegisterMailBox(0xA5A5A5A5);
			if (sender == 0)
			{
				printf("Test Mail System - Failed registering a sender\n");
				return FALSE;
			}
			receiver = RegisterMailBox(0x5A5A5A5A);
			if (receiver == 0)
			{
				printf("Test Mail System - Failed registering a receiver\n");
				ReleaseMailBox(0xA5A5A5A5);
				return FALSE;
			}
			printf("Test Mail System - Sending a Message\n");
			myMessage.sender = 0xA5A5A5A5;
			myMessage.receiver = 0x5A5A5A5A;
			myMessage.messagePtr = messageStr;
			myMessage.size = sizeof(messageStr);
			MutexSpinLock(MUTEX_MAILBOX);
			retVal = SendMessage(myMessage);
			MutexRelease(MUTEX_MAILBOX);
			if (retVal == FALSE)
			{
				printf("Test Mail System - Failed to Send\n");
				ReleaseMailBox(0xA5A5A5A5);
				ReleaseMailBox(0x5A5A5A5A);
				return FALSE;
			}
			tries = 100;
			testStep++;
			break;
		case 1:
			MutexSpinLock(MUTEX_MAILBOX);
			retVal = RetrieveMessage(&myMessage);
			MutexRelease(MUTEX_MAILBOX);
			if (retVal == FALSE)
			{
				tries--;
			}
			else
			{
				printf("Received Message: %s\n", myMessage.messagePtr);
				ReleaseMailBox(0xA5A5A5A5);
				ReleaseMailBox(0x5A5A5A5A);
				return TRUE;
			}
			if (tries == 0)
			{
				printf("Test Mail System - Failed to Receive Message\n");
				ReleaseMailBox(0xA5A5A5A5);
				ReleaseMailBox(0x5A5A5A5A);
				return FALSE;
			}
			break;
		default:
			return FALSE;
		}
	}
}

// EOF
