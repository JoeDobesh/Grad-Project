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

//lines 20

#ifdef __cplusplus
}
#endif

#endif /* INC_GLOBALS_H_ */
