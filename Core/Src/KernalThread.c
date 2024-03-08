/*
 * KernalThread.c
 *
 *  Created on: Feb 16, 2023
 *      Author: jdobesh
 */

#include "Globals.h"
#include "FIFO.h"
#include "KernalThread.h"
#include "UART_3.h"
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
#include "UserApps/CrossingTask.h"
#include "lwip.h"
#include "lwip/apps/httpd.h"
#include "UserApps/PowerControlTask.h"

#define MAX_NUM_OF_USER_TASKS 10
#define TASK_NAME_SIZE 32
#define SYSTICK_VALUE 10

extern struct netif gnetif;

typedef void (*USER_THREAD_PTR)(void);
typedef struct _USER_THREAD
{
	char name[TASK_NAME_SIZE];
	uint32_t taskId;
	USER_THREAD_PTR userThreadPtr;
	BOOL used;
}USER_THREAD;

typedef struct _REGISTERS_
{
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t sp;
	uint32_t lr;
	uint32_t psr;
	uint32_t pc;
}CORE_REGISTERS;
typedef CORE_REGISTERS * CORE_REGS_PTR;

typedef enum _PCB_STATES_
{
	NEW           = 0,
	READY         = 1,
	RUNNING       = 2,
	WAITING       = 3,
	TERMINATED    = 4,
	UNKNOWN_STATE = 5
}PCB_STATES;

typedef struct _PCB_
{
	PCB_STATES     state;
	CORE_REGISTERS registers;
	USER_THREAD    process;
}PCB;

uint32_t * pxCurrentTCB;
//static PCB pcbArray[MAX_NUM_OF_USER_TASKS];
static USER_THREAD userThreadArray[MAX_NUM_OF_USER_TASKS];
static uint8_t pcbIndex = 0;
static uint8_t pcbMax = 0;
static uint32_t contextCounter = 0;
//*****************************************************************************
// KernalThreadInit
//*****************************************************************************
static void KernalThreadInit(void)
{
	for(int i = 0; i < MAX_NUM_OF_USER_TASKS; i++)
	{
		userThreadArray[i].used = FALSE;
		userThreadArray[i].userThreadPtr = NULL;
	}
	contextCounter = 0;
	printf("KernalThreadInit - Passed\n");
}

//*****************************************************************************
// TimeToContextSwitch
//*****************************************************************************
BOOL TimeToContextSwitch(void)
{
	contextCounter++;
	if(contextCounter >= SYSTICK_VALUE)
	{
		contextCounter = 0;
		return TRUE;
	}

	return FALSE;
}

void vTaskSwitchContext(void)
{

}

//*****************************************************************************
// SaveRegisters
//*****************************************************************************
/*
static void SaveRegisters(CORE_REGS_PTR regs)
{
	register uint32_t r0 asm("r0"); //Scratch Register
	regs->r0 = r0;
	register uint32_t r1 asm("r1"); //Scratch Register
	regs->r1 = r1;
	register uint32_t r2 asm("r2"); //Scratch Register
	regs->r2 = r2;
	register uint32_t r3 asm("r3"); //Scratch Register
	regs->r3 = r3;
	register uint32_t r4 asm("r4"); //Variable Register
	regs->r4 = r4;
	register uint32_t r5 asm("r5"); //Variable Register
	regs->r5 = r5;
	register uint32_t r6 asm("r6"); //Variable Register
	regs->r6 = r6;
	register uint32_t r7 asm("r7"); //Variable Register
	regs->r7 = r7;
	register uint32_t r8 asm("r8"); //Variable Register
	regs->r8 = r8;
	register uint32_t r9 asm("r9"); //Platform Register (Variable)
	regs->r9 = r9;
	register uint32_t r10 asm("r10"); //Variable Register
	regs->r10 = r10;
	register uint32_t r11 asm("r11"); //Variable Register
	regs->r11 = r11;
	register uint32_t r12 asm("r12"); //IP Intra-proceedure-call scratch register
	regs->r12 = r12;
	register uint32_t sp asm("sp"); //Stack Pointer (r13)
	regs->sp = sp;
	register uint32_t lr asm("lr"); //Link Register (r14, Return Address)
	regs->lr = lr;
	asm("mov %0, pc" : "=r"(regs->pc) : : ); //Program Counter (r15)
}
*/
//*****************************************************************************
// RestoreRegisters
//*****************************************************************************
/*
static void RestoreRegisters(CORE_REGS_PTR regs)
{
	register uint32_t r0 asm("r0"); //Scratch Register
	r0 = regs->r0;
	//uint32_t t = regs->r0;
	//asm("mov r0, (t)");
	register uint32_t r1 asm("r1"); //Scratch Register
	r1 = regs->r1;
	register uint32_t r2 asm("r2"); //Scratch Register
	r2 = regs->r2;
	register uint32_t r3 asm("r3"); //Scratch Register
	r3 = regs->r3;
	register uint32_t r4 asm("r4"); //Variable Register
	r4 = regs->r4;
	register uint32_t r5 asm("r5"); //Variable Register
	r5 = regs->r5;
	register uint32_t r6 asm("r6"); //Variable Register
	r6 = regs->r6;
	//register uint32_t r7 asm("r7"); //Variable Register
	//r7 = regs->r7;
	register uint32_t r8 asm("r8"); //Variable Register
	r8 = regs->r8;
	register uint32_t r9 asm("r9"); //Platform Register (Variable)
	r9 = regs->r9;
	register uint32_t r10 asm("r10"); //Variable Register
	r10 = regs->r10;
	register uint32_t r11 asm("r11"); //Variable Register
	r11 = regs->r11;
	register uint32_t r12 asm("r12"); //IP Intra-proceedure-call scratch register
	r12 = regs->r12;
	register uint32_t sp asm("sp"); //Stack Pointer
	sp = regs->sp;
	register uint32_t lr asm("lr"); //Link Register (Return Address) (r14)
	lr = regs->lr;
	//asm("mov %0, pc" : "=r"(regs->pc) : : ); //Program Counter
}

static void popStack(CORE_REGS_PTR regs, uint32_t * stackPtr)
{
	uint32_t * localPtr = stackPtr;

	regs->r0  = *(++localPtr); //Scratch Register
	regs->r1  = *(++localPtr); //Scratch Register
	regs->r2  = *(++localPtr); //Scratch Register
	regs->r3  = *(++localPtr); //Scratch register
	regs->r12 = *(++localPtr); //Intra-proceedure-call scratch Register
	regs->lr  = *(++localPtr); //Link Register (Return Address)
	regs->pc  = *(++localPtr); //Program Counter (r15)
	regs->psr = *(++localPtr); //Processor State Register
	//FYI - r13 = Stack Pointer
}
*/

//*****************************************************************************
// KernalTask
//*****************************************************************************
void KernalTask(void)
{
	pcbIndex = 0;
	pcbMax = 0;
	KernalThreadInit();
	SoftTimerInit();
	RealTimeClockInit();
	EventsInit();
	MutexInit();
	FIFO_Init();
	UART_3_Init();
	MailBagInit();
	CommandPromptInit();
	SD_CardInit();
	InitFAT();
	RS485Init();
	ModbusInit();
	//DataLinkInit();
	//CrossingInit();
	HttpdSsiInit();
	//ServerInit();
	HttpdCgiInit();
	//NetComInit();
	PowerControlInit();
	//RunPowerControl();
	while(TRUE)
	{
		HeartbeatTask();
		TimerTask(); //This needs to be removed and run from CommandPrompt\Test
		//MailbagTask();
		CommandPrompt();
		//ModbusPollTask();
		//ServerTask();
		//SendTestTask();
		//ReceiveTestTask();
		//ModbusTask();
		for(int i = 0; i < MAX_NUM_OF_USER_TASKS; i++)
		{
			if(userThreadArray[i].used == TRUE)
			{
				if ( userThreadArray[i].userThreadPtr != NULL)
				{
					userThreadArray[i].userThreadPtr();
				}
			}
		}
		NetComTask();
		ethernetif_input(&gnetif);
		sys_check_timeouts();
	}
}

//*****************************************************************************
// KernalRegister
//*****************************************************************************
BOOL KernalRegister(void * taskPtr)
{
	for(int i = 0; i < MAX_NUM_OF_USER_TASKS; i++)
	{
		if(userThreadArray[i].used == FALSE)
		{
			userThreadArray[i].userThreadPtr = taskPtr;
			userThreadArray[i].used = TRUE;
			return TRUE;
		}
	}

	return FALSE;
}

//*****************************************************************************
// KernalRelease
//*****************************************************************************
BOOL KernalRelease(void * taskPtr)
{
	if(taskPtr == NULL)
	{
		return FALSE;
	}
	for(int i = 0; i < MAX_NUM_OF_USER_TASKS; i++)
	{
		if(userThreadArray[i].userThreadPtr == taskPtr)
		{
			userThreadArray[i].used = FALSE;
			//userThreadArray[i].userThreadPtr = NULL;
			return TRUE;
		}
	}

	return FALSE;
}

//*****************************************************************************
// ContextSwitchInterrupt
//*****************************************************************************
/*
void ContextSwitchInterrupt(void)
{
	CORE_REGISTERS regs;
	uint32_t mysp;
	uint32_t * myspPtr;

	asm("mov %0, sp" : "=r"(mysp) : : );
	myspPtr = (uint32_t *)mysp;
	popStack(&regs, myspPtr);
	printf("Reg r0: 0x%X\n", regs.r0);
	printf("Reg r0: 0x%X\n", regs.r1);
	printf("Reg r0: 0x%X\n", regs.r2);
	printf("Reg r0: 0x%X\n", regs.r3);
	printf("Reg r0: 0x%X\n", regs.r12);
	printf("Reg r0: 0x%X\n", regs.lr);
	printf("Reg r0: 0x%X\n", regs.pc);
	printf("Reg r0: 0x%X\n", regs.psr);

	//disable interrupts
	//__disable_irq();
	//SaveRegisters(&pcbArray[pcbIndex].regs);
	//pcbIndex = (pcbIndex > pcbMax)? 0: pcbIndex++;
	//RestoreRegisters(&pcbArray[pcbIndex].regs);
	//enable interrupts
	//__enable_irq();
}
*/

// EOF
//lines 190
