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

#define SYSTICK_VALUE_MS 		10
#define STACK_ALIGNMENT_MASK	0x00000007

extern struct netif gnetif;
extern uint32_t heartbeatStack[];
extern uint32_t mailbagStack[];
extern uint32_t commandPromptStack[];
extern uint32_t modbusStack[];
extern uint32_t speedControlStack[];
extern uint32_t crossingStack[];
extern uint32_t crossoverStack[];

typedef struct _QUEUES_
{
	PCB *	firstPCB_ptr;
	PCB *	lastPCB_ptr;
} QUEUE;

typedef enum _MODES_
{
	KERNEL_MODE = 0,
	USER_MODE   = 1
}RUNNING_MODES;

static RUNNING_MODES currentMode;
static QUEUE readyQueue;
PCB * pxCurrentTCB;
static uint32_t quantumCounter = 0;
static BOOL csDisable = TRUE;
static uint32_t processIDCounter = 0;
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
// ForceContextSwitch
//*****************************************************************************
void ForceContextSwitch(void)
{
	quantumCounter = SYSTICK_VALUE_MS;
}

//*****************************************************************************
//* PopNextPCB
//*****************************************************************************
static BOOL PopNextPCB(QUEUE * queue, PCB * pcb)
{
	QUEUE * tempQueue;
	PCB * tempPCB = NULL;

	if(queue == NULL)
	{
		pcb = NULL;
		return FALSE;
	}
	tempQueue = queue;
	if(tempQueue->firstPCB_ptr == NULL)
	{
		pcb = NULL;
		return FALSE;
	}
	tempPCB = tempQueue->firstPCB_ptr;
	tempQueue->firstPCB_ptr = tempQueue->firstPCB_ptr->nextPCB_ptr;
	if(tempQueue->firstPCB_ptr == NULL)
	{
		tempQueue->lastPCB_ptr = NULL;
	}
	pcb = tempPCB;

	return TRUE;
}

//*****************************************************************************
// PushLastPCB
//*****************************************************************************
static BOOL PushLastPCB(QUEUE * queue, PCB * pcb)
{
	QUEUE * tempQueue;
	PCB * tempPCB;

	if(queue == NULL)
	{
		return FALSE;
	}
	if(pcb == NULL)
	{
		return FALSE;
	}
	tempQueue = queue;
	tempPCB = pcb;
	if(tempQueue->firstPCB_ptr == NULL)
	{
		tempQueue->firstPCB_ptr = tempPCB;
	}
	else
	{
		tempQueue->lastPCB_ptr->nextPCB_ptr = tempPCB;
	}
	tempQueue->lastPCB_ptr  = tempPCB;
	tempPCB->nextPCB_ptr  = NULL;

	return TRUE;
}

#define INITIAL_XPSR	0x01000000
#define INITIAL_EXEC_RETURN 0xFFFFFFFD
#define portTASK_RETURN_ADDRESS NULL
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
						uint32_t stackSize,
						uint32_t id)
{
	PCB * tempPCB = NULL;
	uint32_t strLength;
	uint32_t temp;

	tempPCB = malloc(sizeof(PCB));
	if ( tempPCB == NULL )
	{
		return FALSE;
	}
	strLength = strlen(name);
	strLength = (strLength <= TASK_NAME_SIZE)? strLength: TASK_NAME_SIZE;
	memcpy(tempPCB->name, name, strLength);
	tempPCB->name[TASK_NAME_SIZE - 1] = '\0';
	tempPCB->process 		= process;
	tempPCB->programCounter = (uint32_t)process;
	tempPCB->processId 		= id;
	tempPCB->nextPCB_ptr 	= NULL;
	temp = ((uint32_t)stackPtr) + (stackSize - 1); //The STM32 stacks are a Full Descending Stacks
	tempPCB->pTopOfStack = (uint32_t *)(temp & (~(uint32_t)STACK_ALIGNMENT_MASK)); //Make sure alignment is 6bit
	tempPCB->stackSize 		= stackSize;
	tempPCB->startOfStack 	= stackPtr; //by address
	tempPCB->endOfStack     = stackPtr + stackSize; //by address
	tempPCB->pTopOfStack = InitialiseStack(tempPCB->pTopOfStack, process);
	if(PushLastPCB(&readyQueue, tempPCB) == FALSE)
	{
		return FALSE;
	}
	tempPCB->state = READY;

	return TRUE;
}

//*****************************************************************************
// vTaskSwitchContext
//*****************************************************************************
void vTaskSwitchContext(void)
{
	//Halt All Interrupts
	PCB * tempPCB = pxCurrentTCB;

	currentMode = KERNEL_MODE;
	if ( tempPCB != NULL )
	{
		PushLastPCB(&readyQueue, tempPCB);
	}
	if(PopNextPCB(&readyQueue, tempPCB) == FALSE)
	{
		//This is really bad
		//reload processes
		printf("Kernal Ready Queue Critical Pop Next Failure!\n");
	}
	else
	{
		pxCurrentTCB = tempPCB;
	}
	quantumCounter = 0;
}

//*****************************************************************************
// KernalThreadInit
//*****************************************************************************
static void KernalThreadInit(void)
{
	currentMode                  = KERNEL_MODE;
	readyQueue.firstPCB_ptr      = NULL;
	readyQueue.lastPCB_ptr       = NULL;
	pxCurrentTCB                 = NULL;
	csDisable					 = TRUE;
	processIDCounter             = 0;
	if(LoadProcess(HeartbeatTask, "Heartbeat\n", heartbeatStack, HEARTBEAT_STACK_SIZE, processIDCounter) == FALSE)
	{
		printf("Mailbag Task Load Failure\n");
	}
	processIDCounter++;
	if(LoadProcess(MailbagTask, "Mailbag\n", mailbagStack, MAILBAG_STACK_SIZE, processIDCounter) == FALSE)
	{
		printf("Mailbag Task Load Failure\n");
	}
	processIDCounter++;
	if(LoadProcess(CommandPrompt, "CommandPrompt\n", commandPromptStack, COMMAND_PROMPT_STACK_SIZE, processIDCounter) == FALSE)
	{
		printf("Command Prompt Task Load Failure\n");
	}
	processIDCounter++;
	if(LoadProcess(ModbusTask, "Modbus\n", modbusStack, MODBUS_STACK_SIZE, processIDCounter) == FALSE)
	{
		printf("Modbus Task Load Failure\n");
	}
	processIDCounter++;
	if(LoadProcess(CrossingTask, "Crossing\n", crossingStack, CROSSING_STACK_SIZE, processIDCounter) == FALSE)
	{
		printf("Crossing Task Load Failure\n");
	}
	processIDCounter++;
	if(LoadProcess(SwitchPowerTask, "Crossover\n", crossoverStack, CROSSOVER_STACK_SIZE, processIDCounter) == FALSE)
	{
		printf("Crossover Task Load Failure\n");
	}
	printf("KernalThreadInit - Passed\n");
}

//*****************************************************************************
// KernalTask
//*****************************************************************************
void KernalTask(void)
{
	BOOL exit = FALSE;

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
	processIDCounter = 0;
	csDisable = FALSE;
	while(exit == FALSE)
	{
		ethernetif_input(&gnetif);
		sys_check_timeouts();
	}
	csDisable = TRUE;
}

// EOF
//lines 190
