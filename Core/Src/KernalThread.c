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
#define SYSTICK_VALUE_MS 			100
#define STACK_ALIGNMENT_MASK		(0x00000007)
#define START_ADDRESS_MASK			(0xFFFFFFFE)
#define INITIAL_XPSR				(0x01000000)
#define INITIAL_EXEC_RETURN 		THREAD_MODE_PROCESS_BASIC
#define TASK_RETURN_ADDRESS 		Oopsy
#define SERVER_TASK_STACK_SIZE		512

extern __IO uint32_t uwTick;
extern HAL_TickFreqTypeDef uwTickFreq;
extern struct netif gnetif;
extern uint32_t mailbagStack[];
extern uint32_t commandPromptStack[];
extern uint32_t modbusStack[];
extern uint32_t speedControlStack[];
extern uint32_t crossingStack[];
extern uint32_t crossoverStack[];

typedef void (*USER_PROCESS_PTR)(void);
typedef struct _QUEUES_
{
	volatile PCB * firstPCB_ptr;
	volatile PCB * lastPCB_ptr;
} QUEUE;

static BOOL enableSwitching = FALSE;
static QUEUE readyQueue;
volatile PCB * pCurrentTCB;
static uint32_t quantumCounter = 0;
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
// Oopsy
//*****************************************************************************
static void Oopsy(void)
{
	printf("Illegal Task Return\n");
	for(;;);
}

void HAL_IncTick(void)
{
  //uwTick += uwTickFreq;
}

uint32_t HAL_GetTick(void)
{
	uint32_t retVal;

	__disable_irq();
	retVal = uwTick;
	__enable_irq();

	return retVal;
}

//*****************************************************************************
// TimeToContextSwitch
//*****************************************************************************
BOOL TimeToContextSwitch(void)
{
	uwTick += uwTickFreq;

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
static volatile uint32_t * InitialiseStack(volatile uint32_t * topOfStack, USER_PROCESS_PTR functionPtr) //, void *pvParameters )
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
	//*pxTopOfStack = pvParameters;								// R0 (Not used)
	// A save method is being used that requires each task to maintain its own exec return value.
	topOfStack--;
	*topOfStack = INITIAL_EXEC_RETURN;
	topOfStack -= 8;											// R11, R10, R9, R8, R7, R6, R5 and R4.

	return topOfStack;
}

//*****************************************************************************
// LoadProcess
//*****************************************************************************
static BOOL LoadProcess(void * process,
						uint32_t * stackPtr,
						PCB * pcb,
						uint32_t stack_size)
{
	uint32_t stackSize = stack_size;
	uint32_t temp;

	assert(process != NULL);
	assert(stackPtr != NULL);
	assert(pcb != NULL);

	memset(stackPtr, 0x00, (stackSize * sizeof(uint32_t)));
	pcb->nextPCB_ptr = NULL;
	pcb->pStackLimit = stackPtr;
	temp = (uint32_t)stackPtr + stackSize;
	pcb->pTopOfStack = (uint32_t *)((temp/8)*8);
	//pcb->pTopOfStack = (uint32_t *)((((uint32_t)stackPtr + stackSize)/8)*8);
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
void vTaskSwitchContext(void) //PRIVILEGED_FUNCTION
{
	volatile PCB * p_tempPCB;

	p_tempPCB = pCurrentTCB;
	if ( p_tempPCB != NULL )
	{
		PushLastPCB(p_tempPCB);
	}
	p_tempPCB = PopNextPCB();
	if(p_tempPCB == NULL)
	{
		printf("Kernal - Ready Queue Critical Pop Next Failure!\n");
	}
	else
	{
		pCurrentTCB = p_tempPCB;
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
	pCurrentTCB             = NULL;
	uint32_t localPSP;

	if(LoadProcess(HeartbeatTask1, &heartbeatStack1[0], &heartbeatPCB1, HEARTBEAT_STACK_SIZE_1) == FALSE)
	{
		printf("Heartbeat 1 Task Load Failure\n");
	}
	if(LoadProcess(HeartbeatTask2, &heartbeatStack2[0], &heartbeatPCB2, HEARTBEAT_STACK_SIZE_2) == FALSE)
	{
		printf("Heartbeat 2 Task Load Failure\n");
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
	localPSP = (uint32_t)(&serverStack[SERVER_TASK_STACK_SIZE - 1]);
	localPSP = (localPSP*8)/8;
	__set_PSP(localPSP);
}

//*****************************************************************************
// KernalTask
//*****************************************************************************
void KernalTask(void)
{
	//enableSwitching = FALSE;
	//__disable_irq();
	//SoftTimerInit();
	MutexInit();
	//FIFO_Init();
	//SD_CardInit();
	//InitFAT();
	//HttpdSsiInit();
	//HttpdCgiInit();
	//RS485Init();
	//ModbusInit();
	//PowerControlInit();
	__disable_irq();
	KernalThreadInit();
	quantumCounter = 0;
	enableSwitching = TRUE;
	__enable_irq();
	while(1){;}
}

// EOF
//lines 190
