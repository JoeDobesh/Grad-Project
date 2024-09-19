/*
 * http_cgi.c
 *
 *  Created on: Dec 5, 2023
 *      Author: joe.dobesh
 */

#include "Globals.h"
#include "UserApps/PowerControlTask.h"
#include "UserApps/SwitchPowerTask.h"
#include "UserApps/CrossingTask.h"
#include "Network/http_cgi.h"
#include "tcp.h"
#include "lwip/apps/httpd.h"

const char *CGISpeedHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char *CGISwitchHandler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char *CGICrossingHandler(int iIndex, int uNumParams, char *pcParam[], char *pcValue[]);

const tCGI SPEED_CGI = {"/speed.cgi", CGISpeedHandler};
const tCGI SWITCH_CGI = {"/switch.cgi", CGISwitchHandler};
const tCGI CROSSING_CGI = {"/crossing.cgi", CGICrossingHandler};
tCGI CGI_TAB[3];

//*****************************************************************************
// HttpdCgiInit
//*****************************************************************************
void HttpdCgiInit(void)
{
	CGI_TAB[0] = SPEED_CGI;
	CGI_TAB[1] = SWITCH_CGI;
	CGI_TAB[2] = CROSSING_CGI;
	http_set_cgi_handlers(CGI_TAB, 3);
}

//*****************************************************************************
// CGISpeedHandler
//*****************************************************************************
const char *CGISpeedHandler(int iIndex, int iNumParams, char * pcParam[], char * pcValue[])
{
	if(iIndex == 0)
	{
		for(int i = 0; i < iNumParams; i++)
		{
			if(strcmp(pcParam[i], "retVal") == 0)
			{
				if(strcmp(pcValue[i], "faster") == 0)
				{
					PowerControlIncrament();
				}
				else if (strcmp(pcValue[i], "slower") == 0)
				{
					PowerControlDecrament();
				}
			}
		}
	}

	return "/SpeedControl.shtml";
}

//*****************************************************************************
// CGISwitchHandler
//*****************************************************************************
const char *CGISwitchHandler(int iIndex, int iNumParams, char * pcParam[], char * pcValue[])
{
	if(iIndex == 1)
	{
		for(int i=0; i<iNumParams; i++)
		{
			if(strcmp(pcParam[i], "retVal") == 0)
			{
				if(strcmp(pcValue[i], "open") == 0)
				{
					OpenSwitch();
				}
				else if(strcmp(pcValue[i], "close") == 0)
				{
					CloseSwitch();
				}
			}
		}
	}

	return "/CrossoverSwitchControl.shtml";
}

//*****************************************************************************
// CGICrossingHandler
//*****************************************************************************
const char *CGICrossingHandler(int iIndex, int iNumParams, char * pcParam[], char * pcValue[])
{
	if(iIndex == 1)
	{
		for(int i=0; i<iNumParams; i++)
		{
			if(strcmp(pcParam[i], "retVal") == 0)
			{
				if(strcmp(pcValue[i], "Crossing") == 0)
				{
					if(strcmp(pcValue[i+1], "CR5") == 0)
					{
						SelectCrossing(0);
					}
					else if(strcmp(pcValue[i+1], "DMS") == 0)
					{
						SelectCrossing(1);
					}
					else if(strcmp(pcValue[i+1], "SH2") == 0)
					{
						SelectCrossing(2);
					}
				}
			}
		}
	}

	return "/CrossingMontor.shtml";
}

//EOF
