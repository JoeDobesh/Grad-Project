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
#include "Disk\SPI.h"
#include "Disk\SD_Card.h"
#include "Disk\FATSystem.h"
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

#define TASK_NAME_SIZE 				32
#define SYSTICK_VALUE_MS 			10
#define STACK_ALIGNMENT_MASK		(0x00000007)
#define START_ADDRESS_MASK			(0xFFFFFFFE)
#define INITIAL_XPSR				(0x01000000)
#define TASK_RETURN_ADDRESS 		KernelTask;
#define SERVER_TASK_STACK_SIZE		(512*4)

int32_t interruptDisableCounter;

extern IWDG_HandleTypeDef hiwdg;

extern struct netif gnetif;
extern uint32_t modbusStack[];
extern uint32_t crossingStack[];
extern uint32_t crossoverStack[];

typedef void (*USER_PROCESS_PTR)(void);
typedef struct _QUEUES_
{
	volatile PCB * firstPCB_ptr;
	volatile PCB * lastPCB_ptr;
} QUEUE;

PCB kernelPCB;
uint32_t kernelStack[64];
static BOOL enableSwitching = FALSE;
static QUEUE readyQueue;
volatile PCB * pCurrentPCB;
static uint32_t quantumCounter = 0;
static uint32_t serverStack[SERVER_TASK_STACK_SIZE];
//*****************************************************************************
// ServerTask
//*****************************************************************************
static void ServerTask(void)
{
	while(TRUE)
	{
		ethernetif_input(&gnetif);
		sys_check_timeouts();
	}
}

//*****************************************************************************
// KernelTask
//*****************************************************************************
void KernelTask(void)
{
	printf("Entering Kernel Task\n");
	for(;;);
}

//*****************************************************************************
// TimeToContextSwitch
//*****************************************************************************
BOOL TimeToContextSwitch(void)
{
	//HAL_IWDG_Refresh(&hiwdg);
	if(enableSwitching == FALSE)
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
static volatile PCB * PopNextPCB(void)
{
	volatile PCB * p_tempPCB = NULL;

	if(readyQueue.firstPCB_ptr == NULL)
	{
		return NULL;
	}
	p_tempPCB = readyQueue.firstPCB_ptr;
	readyQueue.firstPCB_ptr = readyQueue.firstPCB_ptr->nextPCB_ptr;
	if(readyQueue.firstPCB_ptr == NULL)
	{
		readyQueue.lastPCB_ptr = NULL;
	}

	return p_tempPCB;
}

//*****************************************************************************
// PushLastPCB
//*****************************************************************************
static BOOL PushLastPCB(volatile PCB * pcb)
{
	volatile PCB * p_tempPCB = pcb;

	if(pcb == NULL)
	{
		return FALSE;
	}
	if(readyQueue.firstPCB_ptr == NULL)
	{
		readyQueue.firstPCB_ptr             = p_tempPCB;
		readyQueue.lastPCB_ptr              = p_tempPCB;
		readyQueue.lastPCB_ptr->nextPCB_ptr = NULL;
	}
	else
	{
		readyQueue.lastPCB_ptr->nextPCB_ptr = p_tempPCB;
		readyQueue.lastPCB_ptr              = p_tempPCB;
		readyQueue.lastPCB_ptr->nextPCB_ptr = NULL;
	}

	return TRUE;
}

//*****************************************************************************
// InitialiseStack
//*****************************************************************************
static volatile uint32_t * InitialiseStack(volatile uint32_t * topOfStack, USER_PROCESS_PTR functionPtr)
{
	//Simulate the stack frame as it would be created by a context switch interrupt.
	//Offset added to account for the way the MCU uses the stack on entry/exit of interrupts,
	//and to ensure alignment.
	topOfStack--;
	*topOfStack = INITIAL_XPSR;									// xPSR
	topOfStack--;
	*topOfStack = ((uint32_t)functionPtr); // & START_ADDRESS_MASK;	// PC (R15)
	topOfStack--;
	*topOfStack = (uint32_t)TASK_RETURN_ADDRESS;				// LR (R14)
	// Save code space by skipping register initialization.
	topOfStack -= 5;											// R12 (IP), R3, R2 and R1.
	// A save method is being used that requires each task to maintain its own exec return value.
	topOfStack--;
	*topOfStack = THREAD_MODE_PSP;
	topOfStack -= 8;											// R11, R10, R9, R8, R7, R6, R5 and R4.

	return topOfStack;
}

//*****************************************************************************
// LoadProcess
//*****************************************************************************
static BOOL LoadProcess(void * process,
						uint32_t * stackPtr,
						//PCB * pcb,
						uint32_t stack_size)
{
	uint32_t stackSize = stack_size;
	uint32_t temp;

	assert(process != NULL);
	assert(stackPtr != NULL);
	//assert(pcb != NULL);

	PCB * pcb;

	pcb = malloc(sizeof(PCB));
	if(pcb == NULL)
	{
		return FALSE;
	}
	memset(stackPtr, 0x00, (stackSize * sizeof(uint32_t)));
	pcb->nextPCB_ptr = NULL;
	pcb->pStackLimit = stackPtr;
	temp = (uint32_t)stackPtr + stackSize;
	pcb->pTopOfStack = (uint32_t *)((temp/8)*8);
	pcb->pTopOfStack = InitialiseStack(pcb->pTopOfStack, process);
	if(PushLastPCB(pcb) == FALSE)
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
	volatile PCB * p_tempPCB;

	p_tempPCB = &kernelPCB;
	//if ( p_tempPCB != NULL )
	if (p_tempPCB != pCurrentPCB)
	{
		PushLastPCB(pCurrentPCB);
	}
	p_tempPCB = PopNextPCB();
	if(p_tempPCB == NULL)
	{
		printf("Kernal - Ready Queue Critical Pop Next Failure!\n");
	}
	else
	{
		pCurrentPCB = p_tempPCB;
	}
	quantumCounter = 0;
}

//*****************************************************************************
// KernalThreadInit
//*****************************************************************************
static void KernalThreadInit(void)
{
	readyQueue.firstPCB_ptr = NULL;
	readyQueue.lastPCB_ptr  = NULL;
	volatile PCB * p_tempPCB = &kernelPCB;

	if(LoadProcess(HeartbeatTask1, &heartbeatStack1[0], /*&heartbeatPCB1,*/ HEARTBEAT_STACK_SIZE_1) == FALSE)
	{
		printf("Heartbeat 1 Task Load Failure\n");
	}
	if(LoadProcess(HeartbeatTask2, &heartbeatStack2[0], /*&heartbeatPCB2,*/ HEARTBEAT_STACK_SIZE_2) == FALSE)
	{
		printf("Heartbeat 2 Task Load Failure\n");
	}
	if(LoadProcess(MailbagTask, &mailbagStack[0], /*&mailBagPCB,*/ MAILBAG_STACK_SIZE) == FALSE)
	{
		printf("Mailbag Task Load Failure\n");
	}
	if(LoadProcess(CommandPrompt, &commandPromptStack[0], /*&commandPromptPCB,*/ COMMAND_PROMPT_STACK_SIZE) == FALSE)
	{
		printf("Command Prompt Task Load Failure\n");
	}
//	if(LoadProcess(ServerTask, &serverStack[0], /*&serverPCB,*/ SERVER_TASK_STACK_SIZE) == FALSE)
//	{
//		printf("Server Task Load Failure\n");
//	}
	if(LoadProcess(ModbusTask, &modbusStack[0], MODBUS_STACK_SIZE) == FALSE)
	{
		printf("Modbus Task Load Failure\n");
	}
	if(LoadProcess(CrossingTask, &crossingStack[0], CROSSING_STACK_SIZE) == FALSE)
	{
		printf("Crossing Task Load Failure\n");
	}
	/*
	if(LoadProcess(SwitchPowerTask, &crossoverStack[0], CROSSOVER_STACK_SIZE) == FALSE)
	{
		printf("Crossover Task Load Failure\n");
	}
	*/
	memset(kernelStack, 0x00, (64 * sizeof(uint32_t)));
	p_tempPCB->nextPCB_ptr = NULL;
	p_tempPCB->pStackLimit = kernelStack;
	uint32_t temp = (uint32_t)kernelStack + 64;
	p_tempPCB->pTopOfStack = (uint32_t *)((temp/8)*8);
	pCurrentPCB = p_tempPCB;
}

//*****************************************************************************
// KernalTask
//*****************************************************************************
void KernalTask(void)
{
	DisableAllInterrupts();
	enableSwitching = FALSE;
	MutexInit();
	FIFO_Init();
	MailBagInit();
	SoftTimerInit();
	RS485Init();
	EnableAllInterrupts();
	if( SD_CardInit() != sdcVALID)
	{
		printf("SD Card Init Failed\n");
	}
	else
	{
		if( InitFAT() != TRUE )
		{
			printf("FAT32 Init Failed\n");
		}
	}
	DisableAllInterrupts();
	HttpdSsiInit();
	HttpdCgiInit();
	KernalThreadInit();
	quantumCounter = 0;
	enableSwitching = TRUE;
	EnableAllInterrupts();
	while(1){;}
}

// EOF
//lines 190
