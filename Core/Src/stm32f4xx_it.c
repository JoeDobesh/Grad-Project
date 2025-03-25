/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f4xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Globals.h"
#include "KernalThread.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//#define configMAX_SYSCALL_INTERRUPT_PRIORITY 4
#define configPRIO_BITS 4
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
extern PCB * volatile pCurrentPCB;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern DMA_HandleTypeDef hdma_uart4_rx;
extern UART_HandleTypeDef huart4;
extern UART_HandleTypeDef huart3;
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Pre-fetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */
    /* This is a naked function. */
    __asm volatile
        (
        "       cpsid	i                       \n" /* disable interrupts */
        "                                       \n"
        "       tst 	lr, 0x04                \n" /* Test if we are in kernel or thread mode */
        "       ite 	eq                      \n"
        "       mrseq	r0, msp                 \n" /* If Kernal mode or Thread is using MSP, save MSP to r0 */
        "       mrsne	r0, psp                 \n" /* If Thread mode and Thread is not using MSP, save PSP to r0 */
        "                                       \n"
        "       isb                             \n" /* flush instruction pipeline */
        "                                       \n"
        "       tst 	lr, 0x10                \n" /* Is the task using the FPU context? */
        "       it 		eq                      \n"
        "       vstmdbeq r0!, {s16-s31}         \n" /* If so, push high vfp registers. May not work in thumb2 mode*/
        "                                       \n"
        "       stmdb	r0!, {r4-r11, lr}       \n" /* Save the core registers. */
        "                                       \n"
        "       tst 	lr, 0x04                \n" /* Test if we are in Kernel mode or Thread mode */
        "       it		eq                      \n"
        "       msreq 	msp, r0                 \n" /* If so, move contents pointed to by r0 */
        "                                       \n"
        "       ldr     r1, =pCurrentPCB        \n" /* Get the location of the current TCB and load the pointer into r1. */
        "       ldr     r2, [r1]                \n" /* load the first TCB item value into r2, Should be a uint32_t pointer (pTopOfStack)*/
        "       str 	r0, [r2]                \n" /* Save the new top of stack into the first member of the TCB. */
        "                                       \n"
        "       push {lr}                       \n"
        "       bl vTaskSwitchContext           \n" /* Call Context Switcher */
        "       pop {lr}                        \n"
        "                                       \n"
        "       ldr		r1, =pCurrentPCB        \n" /* Load the pointer to the current PCB which also points to the stack pointer */
        "       ldr		r1, [r1]                \n" /* The first item in pxCurrentTCB is the pointer to the top of stack. */
        "       ldr		r0, [r1]                \n"
        "                                       \n"
        "       ldmia	r0!, {r4-r11, lr}       \n" /* Pop the core registers. */
        //"       ldm		r0!, {r4-r11, lr}       \n" /* This superseads above. Pop the core registers. */
        "                                       \n"
        "       tst 	lr, 0x10                \n" /* Is the task using the FPU context?  If so, pop the high vfp registers too. */
        "       it 		eq                      \n"
        "       vldmiaeq r0!, {s16-s31}         \n"
        "                                       \n"
        "       mrs 	r1, control             \n"
        "                                       \n"
        "       tst 	lr, 0x04                \n"
        "       ittee 	eq                      \n" /* If Kernel mode or Thread mode is using MSP, zero flag = 1 */
        "       biceq 	r1, 0x03                \n" /* Kernel mode or Thread mode using MSP privileged */
        "       msreq 	msp, r0                 \n"
        "       orrne 	r1, 0x02                \n"
        "       msrne 	psp, r0                 \n" /* load the new top of stack onto PSP */
        "                                       \n"
        "       msr 	control, r1             \n"
        "                                       \n"
        "       isb                             \n"
        "                                       \n"
        "       cpsie	i                       \n"
        "                                       \n"
        "       bx		lr                      \n" /* jump to next task */
        "                                       \n"
        "       .align 4                        \n"
//        "pCurrentPCBConst: .word pCurrentPCB    \n"
//        ::"i"(configMAX_SYSCALL_INTERRUPT_PRIORITY)
    );
  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */
  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */
  DisableAllInterrupts();
  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */
  if(TimeToContextSwitch() == TRUE)
  {
	  //trigger PendSV
	  SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
  }
  //Enable interrupts
  EnableAllInterrupts();
  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles DMA1 stream2 global interrupt.
  */
void DMA1_Stream2_IRQHandler(void)
{
  /* USER CODE BEGIN DMA1_Stream2_IRQn 0 */

  /* USER CODE END DMA1_Stream2_IRQn 0 */
  HAL_DMA_IRQHandler(&hdma_uart4_rx);
  /* USER CODE BEGIN DMA1_Stream2_IRQn 1 */

  /* USER CODE END DMA1_Stream2_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
  /* USER CODE BEGIN TIM2_IRQn 0 */

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */

  /* USER CODE END TIM2_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
  /* USER CODE BEGIN TIM3_IRQn 0 */

  /* USER CODE END TIM3_IRQn 0 */
  HAL_TIM_IRQHandler(&htim3);
  /* USER CODE BEGIN TIM3_IRQn 1 */

  /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
  /* USER CODE BEGIN USART3_IRQn 0 */

  /* USER CODE END USART3_IRQn 0 */
  HAL_UART_IRQHandler(&huart3);
  /* USER CODE BEGIN USART3_IRQn 1 */

  /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles UART4 global interrupt.
  */
void UART4_IRQHandler(void)
{
  /* USER CODE BEGIN UART4_IRQn 0 */

  /* USER CODE END UART4_IRQn 0 */
  HAL_UART_IRQHandler(&huart4);
  /* USER CODE BEGIN UART4_IRQn 1 */

  /* USER CODE END UART4_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
