/*
 * Globals.h
 *
 *  Created on: Dec 5, 2022
 *      Author: jdobesh
 */
#ifndef INC_GLOBALS_H_
#define INC_GLOBALS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#include <unistd.h>
#include "stm32f4xx_hal.h"
#include "main.h"

#define FALSE (0)
#define TRUE (!FALSE)
#define OFF (0)
#define ON (1)

#define DEBOUNCE_TIME 10

typedef unsigned char BOOL;
typedef int HANDLE;

typedef enum _STATES {
	stateZero = 0,
	stateOne,
	stateTwo,
	stateThree,
	stateFour,
	stateFive,
	stateSix,
	stateSeven,
	stateEight,
	stateNine,
	stateTen
} STATES;

#define KERNEL_MAILBOX_ID			0
#define SERVER_MAILBOX_ID			1
#define SPEED_CONTROL_MAILBOX_ID 	2
#define CROSSING_GATE_MAILBOX_ID	3
#define CROSSOVER_SWITCH_MAILBOX_ID	4

#define KERNEL_PIPELINE_ID				0
#define SERVER_PIPELINE_ID				1
#define SPEED_CONTROL_PIPELINE_ID		2
#define CROSSING_GATE_PIPELINE_ID		3
#define CROSSOVER_SWITCH_PIPELINE_ID	4

//lines 20

#ifdef __cplusplus
}
#endif

#endif /* INC_GLOBALS_H_ */
