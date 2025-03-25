/*
 * CommandPrompt.h
 *
 *  Created on: Jan 23, 2023
 *      Author: jdobesh
 */

#ifndef INC_COMMANDPROMPT_H_
#define INC_COMMANDPROMPT_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"
#include "KernalThread.h"

#define COMMAND_PROMPT_STACK_SIZE 1000

extern uint32_t commandPromptStack[COMMAND_PROMPT_STACK_SIZE];

extern PCB commandPromptPCB;

void CommandPrompt(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_COMMANDPROMPT_H_ */

//lines 3
