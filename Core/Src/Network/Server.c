/*
 * Server.c
 *
 *  Created on: Jul 20, 2023
 *      Author: joe.dobesh
 */

#include "Network\Server.h"
#include "KernalThread.h"

#define BUFFER_SIZE 30000
#define IP1 192
#define IP2 168
#define IP3	0
#define IP4 111

extern char HTML_index[];
extern char HTML_speedcontrol[];
extern char HTML_loopcontrol[];

char header[] = {"HTTP/1.1 200 OK\nContent-Type: text/plane\nContent-Length: 12\n\nHello World!"};
//char header[] = {"HTTP/1.1 200 OK\nContent-Type: text/html;charset=UTF-8\nContent-Length: 1846\n\n"};

static char indexPage[] = {" \
<!DOCTYPE html> \
<html lang=\"en-US\"> \
<head> \
<title>Railroad Engineer</title> \
</head> \
<body style=\"background-color:brown;\"> \
<h1 style=\"text-align:center;\">Home Page</h1> \
<hr> \
<button onclick=\"document.location=\'SpeedControl.html\'\">Speed Control Page</button> \
<br> \
<br> \
<a href=\"SpeedControl.html\"> \
<img src=\"WebPageFinal.jpg\" alt=\"Power Control Image\" style=\"width:128px;height:128px;\"> \
</a> \
<p title=\"Figure 1\"><b>Click to go to speed control page</b></p> \
<h1 style=\"background-color:gray;\">This is heading 1</h1> \
<p style=\"color:yellow;font-family:verdana;\">This is some test</p> \
<hr> \
<h2 style=\"text-align:center;\">This is some heading 2</h2> \
<p style=\"background-color:green;color:blue;font-size:50px;\">This is some other test</p> \
<hr> \
</body> \
</html> \
"};

#define AF_INET 0
#define SOCK_STREAM 0

//static const uint32_t domain = AF_INET;
//static const uint32_t type = SOCK_STREAM; //SOCK_DGRAM SOCK_RAW
//static const uint32_t protocol = 0;
//static const uint32_t myPortNumber = 8080;
//static const uint32_t maxConnections = 3;

//static SOCK_ADDRESS address;
//static const uint32_t address_len = sizeof(address);
//static uint32_t newSocket;
//static struct tcp_pcb *tpcb;
static ip_addr_t myIPADDR;

typedef enum _tcp_server_states_
{
	ES_NONE = 0,
	ES_ACCEPTED,
	ES_RECEIVED,
	ES_CLOSING
}tcp_server_states;

struct tcp_server_struct
{
	uint8_t state;
	uint8_t retries;
	struct tcp_pcb *pcb;
	struct pbuf *p;
};

typedef enum _METHODS
{
	HTTP = 0,
	GET,
	HEAD,
	PUT,
	POST,
	DELETE,
	METHOD_UNKNOWN
} METHODS;

typedef enum _MIME
{
	BMP = 0,
	CSS,
	HTML,
	ICON,
	JPEG,
	JS,
	JSON,
	MP3,
	PNG,
	TIF,
	TXT,
	WAV,
	XHTML,
	XML,
	MIME_UNKNOWN
} MIME;

//static char request[100] = {0};
static char buffer[BUFFER_SIZE] = {0};

//*****************************************************************************
// GetClientMethod
//*****************************************************************************
METHODS GetClientMethod(char * buffer)
{
	//uint32_t index = 0;
	char *temp = buffer;

	if(temp[0] == 'G')
	{
		return GET;
	}
	else if(temp[0] == 'H')
	{
		if(temp[1] == 'T')
		{
			return HTTP;
		}
		else if (temp[1] == 'E')
		{
			return HEAD;
		}
		else
		{
			return METHOD_UNKNOWN;
		}
	}
	else if (temp[0] == 'P')
	{
		if (temp[1] == 'U')
		{
			return PUT;
		}
		else if (temp[1] == 'O')
		{
			return POST;
		}
		else
		{
			return METHOD_UNKNOWN;
		}
	}
	else if (temp[0] == 'D')
	{
		return DELETE;
	}
	else
	{
		return METHOD_UNKNOWN;
	}
}

//*****************************************************************************
// GetMIMEType
//*****************************************************************************
MIME GetMIMEType(char * buffer, char * MIME_string)
{
	char * temp = buffer;
	uint32_t index = 0;

	while (temp[index] != ' ') index++;
	index++;
	while(temp[index] != '.')
	{
		if(temp[index] == ' ')
		{
			char str1[] = {"/"};
			memcpy(MIME_string, str1, sizeof(str1));
			return TXT;
		}
	}
	if ((temp[index] == 'b') && (temp[index + 1] == 'm') && (temp[index + 2] == 'p'))
	{
		char str1[] = {"image/bmp"};
		memcpy(MIME_string, str1, sizeof(str1));
		return BMP;
	}
	else if ((temp[index] == 'c') && (temp[index + 1] == 's') && (temp[index + 2] == 's'))
	{
		char str1[] = {"text/css"};
		memcpy(MIME_string, str1, sizeof(str1));
		return CSS;
	}
	else if ((temp[index] == 'h') && (temp[index + 1] == 't') && (temp[index + 2] == 'm'))
	{
		char str1[] = {"text/html"};
		memcpy(MIME_string, str1, sizeof(str1));
		return HTML;
	}
	else if ((temp[index] == 'i') && (temp[index + 1] == 'c') && (temp[index + 2] == 'o'))
	{
		char str1[] = {"image/vnd.microsoft.icon"};
		memcpy(MIME_string, str1, sizeof(str1));
		return ICON;
	}
	else if ((temp[index] == 'j') && (temp[index + 1] == 'p'))
	{
		char str1[] = {"image/jpeg"};
		memcpy(MIME_string, str1, sizeof(str1));
		return JPEG;
	}
	else if ((temp[index] == 'j') && (temp[index + 1] == 's'))
	{
		char str1[] = {"text/javascript"};
		memcpy(MIME_string, str1, sizeof(str1));
		return JS;
	}
	else if ((temp[index] == 'j') && (temp[index + 1] == 's') && (temp[index + 2] == 'o') && (temp[index + 3] == 'n') && (temp[index + 4] != 'l'))
	{
		char str1[] = {"application/json"};
		memcpy(MIME_string, str1, sizeof(str1));
		return JSON;
	}
	else if ((temp[index] == 'm') && (temp[index + 1] == 'p') && (temp[index + 2] == '3'))
	{
		char str1[] = {"audio/mpeg"};
		memcpy(MIME_string, str1, sizeof(str1));
		return MP3;
	}
	else if ((temp[index] == 'p') && (temp[index + 1] == 'n') && (temp[index + 2] == 'g'))
	{
		char str1[] = {"image/png"};
		memcpy(MIME_string, str1, sizeof(str1));
		return PNG;
	}
	else if ((temp[index] == 't') && (temp[index + 1] == 'i') && (temp[index + 2] == 'f'))
	{
		char str1[] = {"image/tiff"};
		memcpy(MIME_string, str1, sizeof(str1));
		return TIF;
	}
	else if ((temp[index] == 't') && (temp[index + 1] == 'x') && (temp[index + 2] == 't'))
	{
		char str1[] = {"text/plain"};
		memcpy(MIME_string, str1, sizeof(str1));
		return TXT;
	}
	else if ((temp[index] == 'w') && (temp[index + 1] == 'a') && (temp[index + 2] == 'v'))
	{
		char str1[] = {"audio/wav"};
		memcpy(MIME_string, str1, sizeof(str1));
		return WAV;
	}
	if ((temp[index] == 'x') && (temp[index + 1] == 'h') && (temp[index + 2] == 't'))
	{
		char str1[] = {"application/xhtml+xml"};
		memcpy(MIME_string, str1, sizeof(str1));
		return XHTML;
	}
	else if ((temp[index] == 'x') && (temp[index + 1] == 'm') && (temp[index + 2] == 'l'))
	{
		char str1[] = {"application/xml"};
		memcpy(MIME_string, str1, sizeof(str1));
		return XML;
	}
	else
	{
		char str1[] = {""};
		memcpy(MIME_string, str1, sizeof(str1));
		return MIME_UNKNOWN;
	}
}

//*****************************************************************************
// GetFileName
//*****************************************************************************
BOOL GetFileName(char * buffer, char * fileName)
{
	char * temp = buffer;
	uint32_t indexIn = 0, indexOut = 0;

	while (temp[indexIn] != ' ') indexIn++;
	indexIn++;
	while(temp[indexIn] != ' ')
	{
		fileName[indexOut++] = temp[indexIn++];
	}
	if(indexOut == 0)
	{
		return FALSE;
	}

	return TRUE;
}

//*****************************************************************************
// AssembleResponse
//*****************************************************************************
BOOL AssembleResponse(char * body, char * sendMsg)
{
	char * temp = body;
	uint32_t index = 0;
	char len[10];

	int bodySize = sizeof(body);
	memcpy(&sendMsg[index], header, sizeof(header));
	index += sizeof(header);
	snprintf(len, sizeof(len), "%d", bodySize);
	memcpy(&sendMsg[index], len, sizeof(len));
	index += sizeof(len);
	memcpy(&sendMsg[index], "\n\n", 2);
	index += 2;
	memcpy(&sendMsg[index], temp, bodySize);

	return TRUE;
}

static void tcp_server_send(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	struct pbuf *ptr;
	err_t wr_err = ERR_OK;

	while((wr_err == ERR_OK) && (es->p != NULL) && (es->p->len <= tcp_sndbuf(tpcb)))
	{
		ptr = es->p;
		wr_err = tcp_write(tpcb, ptr->payload, ptr->len, 1);
		if(wr_err == ERR_OK)
		{
			uint16_t plen;
			plen = ptr->len;
			es->p = ptr->next;
			if(es->p != NULL)
			{
				pbuf_ref(es->p);
			}
			pbuf_free(ptr);
			tcp_recved(tpcb, plen);
		}
		else if(wr_err == ERR_MEM)
		{
			es->p = ptr;
		}
		else
		{

		}
	}
}

static void tcp_server_connection_close(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	tcp_arg(tpcb, NULL);
	tcp_sent(tpcb, NULL);
	tcp_recv(tpcb, NULL);
	tcp_err(tpcb, NULL);
	tcp_poll(tpcb, NULL, 0);
	if(es != NULL)
	{
		mem_free(es);
	}
}

static void tcp_server_handle(struct tcp_pcb *tpcb, struct tcp_server_struct *es)
{
	struct tcp_server_struct *esTx;
	ip4_addr_t inIP = tpcb->remote_ip;
	uint16_t inPort = tpcb->remote_port;
	char *remIP = ipaddr_ntoa(&inIP);
	char buf[100];

	esTx->state = es->state;
	esTx->pcb = es->pcb;
	esTx->p = es->p;
	memset(buf, '\0', 100);
	strncpy(buf, (char *)es->p->payload, es->p->tot_len);
	strcat(buf, "+ Hello from TCP SERVER\n");
	esTx->p->payload = (void *)buf;
	esTx->p->tot_len = (es->p->tot_len - es->p->len) + strlen(buf);
	esTx->p->len = strlen(buf);
	tcp_server_send(tpcb, esTx);
	pbuf_free(es->p);
}

static err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct tcp_server_struct *es;

	LWIP_UNUSED_ARG(len);
	es = (struct tcp_server_struct *)arg;
	if(es->p != NULL)
	{
		tcp_server_send(tpcb, es);
	}
	else
	{
		if(es->state == ES_CLOSING)
		{
			tcp_server_connection_close(tpcb, es);
		}
	}

	return ERR_OK;
}

static err_t tcp_server_recv(void * arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
	struct tcp_server_struct *es;
	err_t ret_err;

	LWIP_ASSERT("arg != NULL", arg != NULL);

	es = (struct tcp_server_struct*)arg;
	if(p == NULL)
	{
		es->state = ES_CLOSING;
		if(es->p == NULL)
		{
			tcp_server_connection_close(tpcb, es);
		}
		else
		{
			tcp_sent(tpcb, tcp_server_sent);
			tcp_server_send(tpcb, es);
		}
		ret_err = ERR_OK;
	}
	else if(err != ERR_OK)
	{
		if(p != NULL)
		{
			es->p = NULL;
			pbuf_free(p);
		}
		ret_err= err;
	}
	else if(es->state == ES_ACCEPTED)
	{
		es->state = ES_RECEIVED;
		es->p = p;
		tcp_sent(tpcb, tcp_server_sent);
		//tcp_server_send(tpcb, es);
		tcp_server_handle(tpcb, es);
		ret_err = ERR_OK;
	}
	else if(es->state == ES_RECEIVED)
	{
		if(es->p == NULL)
		{
			es->p = p;
			//tcp_server_send(tpcb, es);
			tcp_server_handle(tpcb, es);
		}
		else
		{
			struct pbuf *ptr;
			ptr = es->p;
			pbuf_chain(ptr, p);
		}
		ret_err = ERR_OK;
	}
	else if(es->state == ES_CLOSING)
	{
		tcp_recved(tpcb, p->tot_len);
		es->p = NULL;
		pbuf_free(p);
		ret_err = ERR_OK;
	}
	else
	{
		tcp_recved(tpcb, p->tot_len);
		es->p = NULL;
		pbuf_free(p);
		ret_err = ERR_OK;
	}

	return ret_err;
}

static err_t tcp_server_poll(void * arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	struct tcp_server_struct *es;

	es = (struct tcp_server_struct *)arg;
	if(es != NULL)
	{
		if(es->p != NULL)
		{
			tcp_server_send(tpcb, es);
		}
		else
		{
			if(es->state == ES_CLOSING)
			{
				tcp_server_connection_close(tpcb, es);
			}
		}
		ret_err = ERR_OK;
	}
	else
	{
		tcp_abort(tpcb);
		ret_err = ERR_ABRT;
	}

	return ret_err;
}

static void tcp_server_error(void * arg, err_t err)
{
	struct tcp_server_struct *es;

	LWIP_UNUSED_ARG(err);
	es = (struct tcp_server_struct *)arg;
	if(es != NULL)
	{
		mem_free(es);
	}
}

static err_t tcp_server_accept(void *arg, struct tcp_pcb *newpcb, err_t err)
{
	err_t ret_err;
	struct tcp_server_struct *es;

	LWIP_UNUSED_ARG(arg);
	LWIP_UNUSED_ARG(err);
	tcp_setprio(newpcb, TCP_PRIO_MIN);
	es = (struct tcp_server_struct *)mem_malloc(sizeof(struct tcp_server_struct));
	if( es != NULL)
	{
		es->state 	= ES_ACCEPTED;
		es->pcb   	= newpcb;
		es->retries = 0;
		es->p		= NULL;
		tcp_arg(newpcb, es);
		tcp_recv(newpcb, tcp_server_recv);
		tcp_err(newpcb, tcp_server_error);
		tcp_poll(newpcb, tcp_server_poll, 0);
		ret_err = ERR_OK;
	}
	else
	{
		tcp_server_connection_close(newpcb, es);
		ret_err = ERR_MEM;
	}

	return ret_err;
}

//*****************************************************************************
// ServerInit
//*****************************************************************************
BOOL ServerInit(void)
{
	struct tcp_pcb *tpcb;
	err_t err;

	tpcb = tcp_new();
	if(tpcb == NULL)
	{
		printf("Cannot Create Socket\n");
		return FALSE;
	}
	IP_ADDR4(&myIPADDR, IP1, IP2, IP3, IP4);
	err = tcp_bind(tpcb, &myIPADDR, 7);
	if(err != ERR_OK)
	{
		memp_free(MEMP_TCP_PCB, tpcb);
		printf("Bind Socket Failed\n");
		return FALSE;
	}
	tpcb = tcp_listen(tpcb);
	tcp_accept(tpcb, tcp_server_accept);
	printf("ServerInit - Passed\n");

	return TRUE;
}

//*****************************************************************************
// ServerTask
//*****************************************************************************
void ServerTask(void)
{
	char sendBuffer[BUFFER_SIZE] = {0};

	//newSocket = AcceptSocket( server_fd, &address, address_len);
	//if ( newSocket == 0 )
	//{
	//	printf("Accept Failure");
	//	return;
	//}
	//uint32_t retVal = ReadSocket( newSocket, buffer, BUFFER_SIZE );
	printf("%s\n", buffer);
	//if ( retVal == 0 )
	//{
	//	printf("No Bytes are there to read");
	//}
	METHODS clientMethod = GetClientMethod(buffer);
	if(clientMethod == METHOD_UNKNOWN)
	{
		//CloseSocket(newSocket);
		return;
	}
	char requestedFile[100];
	if(GetFileName(buffer, requestedFile) == FALSE)
	{
	//	CloseSocket(newSocket);
		return;
	}
	char responseMIME[30];
	MIME clientMIME = GetMIMEType(buffer, responseMIME);
	if ( clientMIME == MIME_UNKNOWN )
	{
		//CloseSocket(newSocket);
		return;
	}
	if ( clientMethod == GET )
	{
		if(clientMIME == TXT)
		{
			AssembleResponse(indexPage, sendBuffer);
		}
	}
	else if ( clientMethod == POST )
	{

	}
	else
	{
		//CloseSocket(newSocket);
		return;
	}
	//WriteSocket(newSocket, sendBuffer, strlen(sendBuffer));
	//CloseSocket(newSocket);
}

//EOF
//lines 300
