/*
 * CommandPrompt.c
 *
 *  Created on: Jan 23, 2023
 *      Author: jdobesh
 */

#include "FIFO.h"
#include "RS485.h"
#include "CommandPrompt.h"
#include "SoftTimers.h"
#include "UART_3.h"
#include "Modbus.h"
#include "Boot.h"
#include "Directories.h"
#include "FileSystem.h"
#include "UserApps/PowerControlTask.h"

#define INPUT_BUFFER_SIZE 256
#define MAX_FUNCTION_COUNT 4

extern UART_HandleTypeDef huart3;
extern DISK myDisk;

static uint8_t cpState, testState;
static uint8_t runState, sdState;
static char inputBuffer[INPUT_BUFFER_SIZE];

static const char mainMenu[] = {
"\n \
Help Menu\n \
help - Displays this menu\n \
run - Sends you to the railroad manual run menu\n \
sd - Mounts the SD Card\n \
test - Sends you to the driver test menu\n \
\n"
};

static const char runMenu[] = {
"\n \
Enter the number of the feature you would like to override\n \
Return To The Main Menu  - 0\n \
Run Speed Control        - 1\n \
Run Crossing Control     - 2\n \
Run Switch Power Control - 3\n \
\n"
};

static const char sdMenu[] = {
"\n \
Valid Commands\n \
cd / - return to main menu\n \
ls -  List the files in the directory\n \
cat <file> - Display the selected file \
\n"
};

static const char testMenu[] = {
"\n \
Test Menu\n \
Enter the number of the test you would like to run\n \
Return To Main Menu - 0\n \
Soft Timer Test     - 1\n \
Pipeline Test       - 2\n \
Check Boot Sector   - 3\n \
Check RS-485        - 4\n \
Check Modbus        - 5\n \
\n"
};

static void SDCardTask(void);
static void RunMenuTask(void);
static void TestMenuTask(void);
static void MainMenuTask(void);

typedef void (*FunctionPtr)(void);

static FunctionPtr functionArray[] = {
	&MainMenuTask,
	&RunMenuTask,
	&SDCardTask,
	&TestMenuTask
};

static uint8_t functionPtrIndex;

//*****************************************************************************
// JumpToMain
//*****************************************************************************
static void JumpToMain(void)
{
	functionPtrIndex = 0;
	cpState = 0;
}

//*****************************************************************************
// JumpToRun
//*****************************************************************************
static void JumpToRun(void)
{
	functionPtrIndex = 1;
	runState = 0;
}

//*****************************************************************************
// JumpToSD
//*****************************************************************************
static void JumpToSD(void)
{
	functionPtrIndex = 2;
	sdState = 0;
}

//*****************************************************************************
// JumpToTest
//*****************************************************************************
static void JumpToTest(void)
{
	functionPtrIndex = 3;
	testState = 0;
}

//*****************************************************************************
// CommandPromptInit
//*****************************************************************************
BOOL CommandPromptInit(void)
{
	functionPtrIndex = 0;
	cpState = 0;
	runState = 0;
	sdState = 0;
	testState = 0;
	printf("CommandPromptInit - Passed\n");

	return TRUE;
}

//*****************************************************************************
// CommandPrompt
//*****************************************************************************
void CommandPrompt(void)
{
	if ( functionPtrIndex < MAX_FUNCTION_COUNT)
	{
		(*functionArray[functionPtrIndex])();
	}
	else
	{
		JumpToMain();
	}
}

//*****************************************************************************
// MainMenuTask
//*****************************************************************************
static void MainMenuTask(void)
{
	char str1[] = "Grad Project>>";
	static uint32_t inputBuffCounter;
	HAL_StatusTypeDef status;
	size_t size = sizeof(str1);
	char ch;

	switch(cpState)
	{
	case 0:
		UART_3_SendString(str1,  size);
		inputBuffCounter = 0;
		cpState++;
		break;
	case 1:
		status = HAL_UART_Receive(&huart3, (uint8_t *)&ch, 1, 10); //HAL_MAX_DELAY);
		if(status == HAL_OK)
		{
			if(ch == '\n')
			{
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffer[inputBuffCounter++] = '\0';
				cpState++;
			}
			else if (ch == 0x08) //Backspace
			{
				ch = '\b';
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffCounter--;
			}
			else if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'z'))
			{
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffer[inputBuffCounter++] = ch;
				inputBuffCounter = (inputBuffCounter >= INPUT_BUFFER_SIZE)? (INPUT_BUFFER_SIZE - 1): inputBuffCounter;
			}
		}
		break;
	case 2:
		if (strcmp(inputBuffer, "help") == 0)
		{
			printf("%s",mainMenu);
		}
		else if (strcmp(inputBuffer, "run") == 0)
		{
			JumpToRun();
		}
		else if (strcmp(inputBuffer, "sd") == 0)
		{
			JumpToSD();
		}
		else if (strcmp(inputBuffer, "test") == 0)
		{
			JumpToTest();
		}
		else
		{
			printf("%s",mainMenu);
		}
		cpState = 0;
		break;
	default:
		cpState = 0;
		break;
	}
}

//*****************************************************************************
// RunMenuTask
//*****************************************************************************
static void RunMenuTask(void)
{
	HAL_StatusTypeDef status;
	char str1[] = "Run>>";
	char ch;
	size_t size = sizeof(str1);
	static uint32_t inputBuffCounter;

	switch(runState)
	{
	case 0:
		printf("%s", runMenu);
		UART_3_SendString(str1,  size);
		inputBuffCounter = 0;
		runState++;
		break;
	case 1:
		status = HAL_UART_Receive(&huart3, (uint8_t *)&ch, 1, 10); //HAL_MAX_DELAY);
		if(status == HAL_OK)
		{
			if(ch == '\n')
			{
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffer[inputBuffCounter++] = '\0';
				runState++;
			}
			else if (ch == 0x08) //Backspace
			{
				ch = '\b';
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffCounter--;
			}
			else if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'z'))
			{
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffer[inputBuffCounter++] = ch;
				inputBuffCounter = (inputBuffCounter >= INPUT_BUFFER_SIZE)? (INPUT_BUFFER_SIZE - 1): inputBuffCounter;
			}
		}
		break;
	case 2:
		if(strcmp(inputBuffer, "0") == 0)
		{
			JumpToMain(); //Return to Main Menu
		}
		else if(strcmp(inputBuffer, "1") == 0)
		{
			char str2[] = "Running Locomotive Speed Control\n";
			UART_3_SendString(str2, sizeof(str2));
			RunPowerControl();
			runState = 3;
		}
		else if(strcmp(inputBuffer, "2") == 0)
		{
			char str3[] = "Running Crossing Gate Control\n";
			UART_3_SendString(str3, sizeof(str3));
			runState = 4;
		}
		else if(strcmp(inputBuffer, "3") == 0)
		{
			char str4[] = "Running Switch Power Control\n";
			UART_3_SendString(str4, sizeof(str4));
			runState = 5;
		}
		else
		{
			runState = 0;
		}
		break;
	case 3:
		if ( GetPowerControlStatus() == FALSE )
		{
			runState = 0;
		}
		break;
	case 4:
		runState = 0;
		break;
	case 5:
		runState = 0;
		break;
	default:
		runState = 0;
		break;
	}
}

//*****************************************************************************
// GetCatFileName
//*****************************************************************************
static void GetCatFileName(char * string, char * s)
{
	uint8_t i;
	uint8_t o = 0;

	for(i = 0; i < 12; i++)
	{
		if(string[i] == '.')
		{
			o++;
			continue;
		}
		s[i-o] = string[i];
		if(string[i] == '\n' || string[i] == '\0')
		{
			break;
		}
	}
}

//*****************************************************************************
// SDCardTask
//*****************************************************************************
static void SDCardTask(void)
{
	HAL_StatusTypeDef status;
	char str1[] = "SD>>";
	char ch;
	size_t size = sizeof(str1);
	static uint32_t inputBuffCounter;
	FILEOBJ foDest, foCompareTo;
	uint8_t * buffer;

	switch(sdState)
	{
	case 0:
		printf("%s", sdMenu);
		UART_3_SendString(str1,  size);
		inputBuffCounter = 0;
		sdState++;
		break;
	case 1:
		status = HAL_UART_Receive(&huart3, (uint8_t *)&ch, 1, 10); //HAL_MAX_DELAY);
		if(status == HAL_OK)
		{
			if(ch == '\n')
			{
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffer[inputBuffCounter++] = '\0';
				sdState++;
			}
			else if (ch == 0x08) //Backspace
			{
				ch = '\b';
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffCounter--;
			}
			else if ((ch >= '.' && ch <= '9') || (ch >= 'A' && ch <= 'z') || (ch == ' ') || (ch == '~'))
			{
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffer[inputBuffCounter++] = ch;
				inputBuffCounter = (inputBuffCounter >= INPUT_BUFFER_SIZE)? (INPUT_BUFFER_SIZE - 1): inputBuffCounter;
			}
		}
		break;
	case 2:
		if(strncmp(inputBuffer, "cd /", 4) == 0)
		{
			JumpToMain(); //Return to Main Menu
		}
		else if(strncmp(inputBuffer, "ls", 2) == 0)
		{
			ListRootFiles(&myDisk);
			sdState = 3;
		}
		else if(strncmp(inputBuffer, "cat", 3) == 0)
		{
			foDest = malloc(sizeof(FILE_DAT));
			if(foDest == NULL)
			{
				printf("foDest: Not enough memory\n");
				sdState = 4;
				break;
			}
			foCompareTo = malloc(sizeof(FILE_DAT));
			if(foCompareTo == NULL)
			{
				printf("foCompareTo: Not enough memory\n");
				free (foDest);
				sdState = 4;
				break;
			}
			GetCatFileName(&inputBuffer[4], foCompareTo->name);
			myDisk.mount = TRUE;
			foDest->disk = &myDisk;
			foDest->dirclus = myDisk.rootCluster;
			if(Filefind(foDest, foCompareTo, 2) == CE_GOOD)
			{
				if(FileOpen(foDest, &foDest->entry, 'r') == CE_GOOD)
				{
					buffer = malloc(foDest->fileSize);
					if(buffer == NULL)
					{
						printf("buffer: Not enough memory\n");
					}
					else
					{
						while(FileRead(foDest, buffer, foDest->fileSize) == CE_GOOD)
						{
							HAL_UART_Transmit(&huart3, buffer, foDest->fileSize, HAL_MAX_DELAY);
						}
						free(buffer);
					}
					FileClose(foDest);
				}
				else
				{
					printf("File access denied\n");
				}
			}
			else
			{
				printf("File not found\n");
			}
			free (foDest);
			free (foCompareTo);
			sdState = 4;
		}
		else
		{
			sdState = 0;
		}
		break;
	case 3:
		sdState = 0;
		break;
	case 4:
		sdState = 0;
		break;
	case 5:
		sdState = 0;
		break;
	default:
		sdState = 0;
		break;
	}
}

//*****************************************************************************
// TestMenuTask
//*****************************************************************************
static void TestMenuTask(void)
{
	HAL_StatusTypeDef status;
	char str1[] = "Test>>";
	char ch;
	size_t size = sizeof(str1);
	static uint32_t inputBuffCounter;

	switch(testState)
	{
	case 0:
		printf("%s", testMenu);
		UART_3_SendString(str1,  size);
		inputBuffCounter = 0;
		testState++;
		break;
	case 1:
		status = HAL_UART_Receive(&huart3, (uint8_t *)&ch, 1, 10); //HAL_MAX_DELAY);
		if(status == HAL_OK)
		{
			if(ch == '\n')
			{
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffer[inputBuffCounter++] = '\0';
				testState++;
			}
			else if (ch == 0x08) //Backspace
			{
				ch = '\b';
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffCounter--;
			}
			else if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'z'))
			{
				HAL_UART_Transmit(&huart3, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
				inputBuffer[inputBuffCounter++] = ch;
				inputBuffCounter = (inputBuffCounter >= INPUT_BUFFER_SIZE)? (INPUT_BUFFER_SIZE - 1): inputBuffCounter;
			}
		}
		break;
	case 2:
		if(strcmp(inputBuffer, "0") == 0)
		{
			JumpToMain(); //Return to Main Menu
		}
		else if(strcmp(inputBuffer, "1") == 0)
		{
			StartTimerTest();
			char * str2 = "Starting SoftTimers Test\n";
			UART_3_SendString(str2, sizeof(str2));
			testState = 4; //Soft Timer Test
		}
		else if(strcmp(inputBuffer, "2") == 0)
		{
			FIFO_Test();
			testState = 5;
		}
		else if(strcmp(inputBuffer, "3") == 0)
		{
			//Read the Boot Sector
			if ( PrintBootSector() == FALSE )
			{
				printf("PrintBootSector - Failed\n");
				testState = 0;
			}
			else
			{
				testState = 6;
			}
		}
		else if(strcmp(inputBuffer, "4") == 0)
		{
			RS485Test();
			testState = 7;
		}
		else if(strcmp(inputBuffer, "5") == 0)
		{
			ReadHoldingRegisters(1, 1, 1);
			testState = 7;
		}
		else
		{
			testState = 0;
		}
		break;
	case 4:
		if (TimerTestComplete() == TRUE)
		{
			testState = 0;
		}
		break;
	case 5:
		testState = 0;
		break;
	case 6:
		testState = 0;
		break;
	case 7:
		testState = 0;
		break;
	default:
		testState = 0;
		break;
	}
}

//EOF

//lines 510
