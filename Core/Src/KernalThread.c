/*
 * KernalThread.c
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#include "Globals.h"
#include "FIFO.h"
#include "KernalThread.h"
#include "CommandPrompt.h"
#include "HeartbeatTask.h"
#include "SoftTimers.h"
#include "MailBag.h"
#include "RealTimeClock.h"
#include "Network/http_ssi.h"
#include "Network/http_cgi.h"
#include "SPI.h"
#include "SD_Card.h"
#include "FATSystem.h"
#include "RS485.h"
#include "Modbus.h"
#include "Mutex.h"
#include "Event.h"
#include "Mutex.h"
#include "lwip.h"
#include "lwip/apps/httpd.h"
#include "UserApps/PowerControlTask.h"
#include "UserApps/CrossingTask.h"
#include "UserApps/SwitchPowerTask.h"

#define TASK_NAME_SIZE 			32
#define SYSTICK_VALUE_MS 		10
#define STACK_ALIGNMENT_MASK	0x00000007
#define INITIAL_XPSR			0x01000000
#define INITIAL_EXEC_RETURN 	0xFFFFFFFD
#define portTASK_RETURN_ADDRESS NULL
#define SERVER_TASK_STACK_SIZE	4096

extern struct netif gnetif;
extern uint32_t heartbeatStack[];
extern uint32_t heartbeatStack2[];
extern uint32_t mailbagStack[];
extern uint32_t commandPromptStack[];
extern uint32_t modbusStack[];
extern uint32_t speedControlStack[];
extern uint32_t crossingStack[];
extern uint32_t crossoverStack[];

typedef void (*USER_PROCESS_PTR)(void);
typedef struct _PCB_
{
	volatile uint32_t * pTopOfStack;
	uint32_t *			startOfStack;
	char 			 	name[TASK_NAME_SIZE];
	void *			 	nextPCB_ptr;
}PCB;

typedef struct _QUEUES_
{
	PCB *	firstPCB_ptr;
	PCB *	lastPCB_ptr;
} QUEUE;

static QUEUE readyQueue;
PCB * pxCurrentTCB;
static uint32_t quantumCounter = 0;
static BOOL csDisable = TRUE;
static uint32_t serverStack[SERVER_TASK_STACK_SIZE];
//*****************************************************************************
// ServerTask
//*****************************************************************************
static void ServerTask(void)
{
	ethernetif_input(&gnetif);
	sys_check_timeouts();
}

//*****************************************************************************
// DisableContext
//*****************************************************************************
void DisableContext(void)
{
	csDisable = TRUE;
}

//*****************************************************************************
// EnableContext
//*****************************************************************************
void EnableContext(void)
{
	csDisable = FALSE;
}

//*****************************************************************************
// TimeToContextSwitch
//*****************************************************************************
BOOL TimeToContextSwitch(void)
{
	if(csDisable == TRUE)
	{
		return FALSE;
	}
	quantumCounter++;
	if(quantumCounter >= SYSTICK_VALUE_MS)
	{
		quantumCounter = 0;
		return TRUE;
	}

	return FALSE;
}

//*****************************************************************************
//* PopNextPCB
//*****************************************************************************
static BOOL PopNextPCB(PCB ** pcb)
{
	PCB * tempPCB = NULL;

	if(readyQueue.firstPCB_ptr == NULL)
	{
		*pcb = NULL;
		return FALSE;
	}
	tempPCB = readyQueue.firstPCB_ptr;
	readyQueue.firstPCB_ptr = readyQueue.firstPCB_ptr->nextPCB_ptr;
	if(readyQueue.firstPCB_ptr == NULL)
	{
		readyQueue.lastPCB_ptr = NULL;
	}
	*pcb = tempPCB;

	return TRUE;
}

//*****************************************************************************
// PushLastPCB
//*****************************************************************************
static BOOL PushLastPCB(PCB * pcb)
{
	PCB * tempPCB = pcb;

	if(pcb == NULL)
	{
		return FALSE;
	}
	if(readyQueue.firstPCB_ptr == NULL)
	{
		readyQueue.firstPCB_ptr             = tempPCB;
		readyQueue.lastPCB_ptr              = tempPCB;
		readyQueue.lastPCB_ptr->nextPCB_ptr = NULL;
	}
	else
	{
		readyQueue.lastPCB_ptr->nextPCB_ptr = tempPCB;
		readyQueue.lastPCB_ptr              = tempPCB;
		readyQueue.lastPCB_ptr->nextPCB_ptr = NULL;
	}

	return TRUE;
}

//*****************************************************************************
// InitialiseStack
//*****************************************************************************
static uint32_t * InitialiseStack( uint32_t * topOfStack, USER_PROCESS_PTR functionPtr) //, void *pvParameters )
{
	//Simulate the stack frame as it would be created by a context switch interrupt.
	//Offset added to account for the way the MCU uses the stack on entry/exit of interrupts,
	//and to ensure alignment.
	topOfStack--;
	*topOfStack = INITIAL_XPSR;									// xPSR
	topOfStack--;
	*topOfStack = (uint32_t)functionPtr;						// PC (R15)
	topOfStack--;
	*topOfStack = (uint32_t)portTASK_RETURN_ADDRESS;			// LR (R14)
	// Save code space by skipping register initialisation.
	topOfStack -= 5;											// R12 (IP), R3, R2 and R1.
	//*pxTopOfStack = pvParameters;								// R0 (Not used)
	// A save method is being used that requires each task to maintain its own exec return value.
	topOfStack--;
	*topOfStack = INITIAL_EXEC_RETURN;
	topOfStack -= 8;	// R11, R10, R9, R8, R7, R6, R5 and R4.

	return topOfStack;
}

//*****************************************************************************
// LoadProcess
//*****************************************************************************
static BOOL LoadProcess(void * process,
						char * name,
						uint32_t * stackPtr,
						uint32_t stackSize)
{
	PCB * tempPCB = NULL;
	uint32_t strLength;
	uint32_t temp;

	tempPCB = malloc(sizeof(PCB));
	if ( tempPCB == NULL )
	{
		return FALSE;
	}
	memset(tempPCB->name, '\0', TASK_NAME_SIZE);
	strLength = strlen(name);
	strLength = (strLength <= TASK_NAME_SIZE)? strLength: TASK_NAME_SIZE;
	memcpy(tempPCB->name, name, strLength);
	tempPCB->name[TASK_NAME_SIZE - 1] = '\0';
	tempPCB->nextPCB_ptr 	= NULL;
	temp = ((uint32_t)stackPtr) + (stackSize - 1); //The STM32 stacks are a Full Descending Stacks
	tempPCB->pTopOfStack = (uint32_t *)(temp & (~(uint32_t)STACK_ALIGNMENT_MASK)); //Make sure alignment is 64 bit
	tempPCB->startOfStack 	= stackPtr; //by address
	tempPCB->pTopOfStack = InitialiseStack(tempPCB->pTopOfStack, process);
	if(PushLastPCB(tempPCB) == FALSE)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// vTaskSwitchContext
//*****************************************************************************
void vTaskSwitchContext(void)
{
	PCB * tempPCB;

	__disable_irq();
	csDisable = TRUE;
	tempPCB = pxCurrentTCB;
	if ( tempPCB != NULL )
	{
		PushLastPCB(tempPCB);
	}
	if(PopNextPCB(&tempPCB) == FALSE)
	{
		printf("Kernal - Ready Queue Critical Pop Next Failure!\n");
	}
	else
	{
		pxCurrentTCB = tempPCB;
	}
	quantumCounter = 0;
	csDisable = FALSE;
    __enable_irq();
}

//*****************************************************************************
// KernalThreadInit
//*****************************************************************************
static void KernalThreadInit(void)
{
	csDisable                    = TRUE;
	readyQueue.firstPCB_ptr      = NULL;
	readyQueue.lastPCB_ptr       = NULL;
	pxCurrentTCB                 = NULL;
	if(LoadProcess(HeartbeatTask, "Heartbeat\n", heartbeatStack, HEARTBEAT_STACK_SIZE) == FALSE)
	{
		printf("Heartbeat Task Load Failure\n");
	}
	if(LoadProcess(HeartbeatTask2, "Heartbeat2\n", heartbeatStack2, HEARTBEAT_STACK_SIZE_2) == FALSE)
	{
		printf("Heartbeat2 Task Load Failure\n");
	}
	/*
	if(LoadProcess(MailbagTask, "Mailbag\n", mailbagStack, MAILBAG_STACK_SIZE) == FALSE)
	{
		printf("Mailbag Task Load Failure\n");
	}
	if(LoadProcess(CommandPrompt, "CommandPrompt\n", commandPromptStack, COMMAND_PROMPT_STACK_SIZE) == FALSE)
	{
		printf("Command Prompt Task Load Failure\n");
	}
	if(LoadProcess(ServerTask, "Server\n", serverStack, SERVER_TASK_STACK_SIZE) == FALSE)
	{
		printf("Command Prompt Task Load Failure\n");
	}
	if(LoadProcess(ModbusTask, "Modbus\n", modbusStack, MODBUS_STACK_SIZE) == FALSE)
	{
		printf("Modbus Task Load Failure\n");
	}
	if(LoadProcess(CrossingTask, "Crossing\n", crossingStack, CROSSING_STACK_SIZE) == FALSE)
	{
		printf("Crossing Task Load Failure\n");
	}
	if(LoadProcess(SwitchPowerTask, "Crossover\n", crossoverStack, CROSSOVER_STACK_SIZE) == FALSE)
	{
		printf("Crossover Task Load Failure\n");
	}
	*/
}

//*****************************************************************************
// KernalTask
//*****************************************************************************
void KernalTask(void)
{
	BOOL exit = FALSE;

	csDisable = TRUE;
	SoftTimerInit();
	MutexInit();
	FIFO_Init();
	SD_CardInit();
	InitFAT();
	HttpdSsiInit();
	HttpdCgiInit();
	RS485Init();
	ModbusInit();
	PowerControlInit();
	KernalThreadInit();
	quantumCounter = 0;
	csDisable = FALSE;
	while(exit == FALSE)
	{
		//ethernetif_input(&gnetif);
		//sys_check_timeouts();
	}
	csDisable = TRUE;
}

// EOF
//lines 190
