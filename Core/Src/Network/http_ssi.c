/*
 * http_ssi.c
 *
 *  Created on: Dec 5, 2023
 *      Author: joe.dobesh
 */

#include "Globals.h"
#include "Network\http_ssi.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/apps/fs.h"
#include "lwip.h"
#include "lwip/tcp.h"
#include "tcp.h"
#include "err.h"
#include "ip_addr.h"
#include "pbuf.h"
#include "udp.h"
#include "lwip/apps/httpd.h"
#include "UserApps/CrossingTask.h"
#include "UserApps/PowerControlTask.h"
#include "UserApps/SwitchPowerTask.h"

char const* TAGCHAR[]={"speed", "crossingImage", "crossingStatus", "crossingSelect", "crossoverSwitch"};
char const** TAGS=TAGCHAR;

//*****************************************************************************
// HttpdSsiInit
//*****************************************************************************
void HttpdSsiInit(void)
{
	http_set_ssi_handler(SsiHandler, TAGS, 5);
}

//*****************************************************************************
// GetCrossingImage
//*****************************************************************************
static char* GetCrossingImage(void)
{
	uint8_t sensors = GetSersorValues();

	sensors &= 0x0F;
	switch (sensors)
	{
	case 0x00:
		return "img/CrossingGateImage.jpg";
		break;
	case 0x01:
		return "img/CrossingGateImageFar1.jpg";
		break;
	case 0x02:
		return "img/CrossingGateImageNear2.jpg";
		break;
	case 0x03:
		return "img/CrossingGateImageFar1Near2.jpg";
		break;
	case 0x04:
		return "img/CrossingGateImageNear1.jpg";
		break;
	case 0x05:
		return "img/CrossingGateImageFar1Near1.jpg";
		break;
	case 0x06:
		return "img/CrossingGateImageNearNear.jpg";
		break;
	case 0x07:
		return "img/CrossingGateImageFar1Near2Near1.jpg";
		break;
	case 0x08:
		return "img/CrossingGateImageFar2.jpg";
		break;
	case 0x09:
		return "img/CrossingGateImageFarFar.jpg";
		break;
	case 0x0A:
		return "img/CrossingGateImageFar2Near2.jpg";
		break;
	case 0x0B:
		return "img/CrossingGateImageFar2Near2Far1.jpg";
		break;
	case 0x0C:
		return "img/CrossingGateImageFar2Near1.jpg";
		break;
	case 0x0D:
		return "img/CrossingGateImageFar1Near1Far2.jpg";
		break;
	case 0x0E:
		return "img/CrossingGateImageFar2Near1Near2.jpg";
		break;
	case 0x0F:
		return "img/CrossingGateImageAll.jpg";
		break;
	default:
		break;
	}

	return "CrossingGateImage.jpg";
}

//*****************************************************************************
// GetCrossingStatus
//*****************************************************************************
static char* GetCrossingStatus(void)
{
	if(GetGateStatus() == GATE_OPEN)
	{
		return "OPEN";
	}

	return "CLOSED";
}

//*****************************************************************************
// GetStreet
//*****************************************************************************
static char* GetStreet(void)
{
	uint8_t temp = GetCrossing();

	switch(temp)
	{
	case 0:
		return "CR5";
		break;
	case 1:
		return "DMS";
		break;
	case 2:
		return "SH2";
		break;
	default:
		break;
	}

	return "CR5";
}

//*****************************************************************************
// GetCrossoverStatus
//*****************************************************************************
static char* GetCrossoverStatus(void)
{
	SWITCH_STATES state = GetSwitchStatus();
	switch(state)
	{
	case SWITCH_OPEN:
		return "green led";
		break;
	case SWITCH_TRANS:
		return "orange led";
		break;
	case SWITCH_CLOSED:
		return "red led";
		break;
	default:
		break;
	}

	return "green led";
}

//*****************************************************************************
// SsiHandler
//*****************************************************************************
uint16_t SsiHandler(int iIndex, char *pcInsert, int iInsertLen)
{
	switch (iIndex)
	{
	case 0: //speed
		sprintf(pcInsert, "%lu", GetPulseWidth());
		return strlen(pcInsert);
		break;
	case 1: //crossingImage
		sprintf(pcInsert, "%s", GetCrossingImage());
		return strlen(pcInsert);
		break;
	case 2: //crossingStatus
		sprintf(pcInsert, "%s", GetCrossingStatus());
		return strlen(pcInsert);
		break;
	case 3: //crossingSelect
		sprintf(pcInsert, "%s", GetStreet());
		return strlen(pcInsert);
		break;
	case 4: //crossoverSwitch
		sprintf(pcInsert, "%s", GetCrossoverStatus());
		return strlen(pcInsert);
		break;
	default:
		break;
	}

	return 0;
}

void NetComInit(void)
{
	//conn = netconn_new(NETCONN_TCP);
	//if(conn == NULL)
	//{
	//	printf("netconn_new failed\n");
	//	return;
	//}
	//err = netconn_bind(conn, IP_ADDR_ANY, 80);
	//if(err != ERR_OK)
	//{
	//	printf("netconn_bind failed\n");
	//	netrconn_free(conn);
	//	conn = NULL;
	//	return;
	//}
}

void NetComTask(void)
{
	//if(conn == NULL)
	//{
	//	return;
	//}
}


//EOF
