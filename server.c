#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>

using namespace std;

#define BUFFER_SIZE 30000

//static const char header[] = {"HTTP/1.1 200 OK\nContent-Type: text/plane\nContent-Length: 12\n\nHello World!"};
static const char header[] = {"HTTP/1.1 200 OK\nContent-Type: text/html;charset=UTF-8\nContent-Length: "};

static const char indexPage[] = {" \
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

static const int domain = AF_INET;
static const int type = SOCK_STREAM; //SOCK_DGRAM SOCK_RAW
static const int protocol = 0;
static const int myPortNumber = 8080;
static const int maxConnections = 3;

static struct sockaddr_in address;
static socklen_t   address_len = sizeof(address);
static int backlog;
static int newSocket;

typedef enum _METHODS
{
	HTTP = 0,
	GET,
	HEAD,
	PUT,
	POST,
	DELETE,
	UNKNOWN
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
	AXML,
	TXML,
	UNKNOWN
}

static char request[100] = {0};
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
			return UNKNOWN;
		}
	}
	else if (temp[0] == 'P')
	{
		if (temp[1] == 'U')
		{
			return = PUT;
		}
		else if (temp[1] == 'O')
		{
			return = POST;
		}
		else
		{
			return UNKNOWN;
		}
	}
	else if (temp[0] == 'D')
	{
		return DELETE;
	}
	else
	{
		return UNKNOWN;
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
		char str1[] = {"image/bmp"}
		memcpy(MIME_string, str1, sizeof(str1));
		return BMP;
	}
	else if ((temp[index] == 'c') && (temp[index + 1] == 's') && (temp[index + 2] == 's'))
	{
		char str1[] = {"text/css"}
		memcpy(MIME_string, str1, sizeof(str1));
		return CSS;
	}
	else if ((temp[index] == 'h') && (temp[index + 1] == 't') && (temp[index + 2] == 'm')) 
	{
		char str1[] = {"text/html"}
		memcpy(MIME_string, str1, sizeof(str1));
		return HTML;
	}
	else if ((temp[index] == 'i') && (temp[index + 1] == 'c') && (temp[index + 2] == 'o'))
	{
		char str1[] = {"image/vnd.microsoft.icon"}
		memcpy(MIME_string, str1, sizeof(str1));
		return ICON;
	}
	else if ((temp[index] == 'j') && (temp[index + 1] == 'p'))
	{
		char str1[] = {"image/jpeg"}
		memcpy(MIME_string, str1, sizeof(str1));
		return JPEG;
	}
	else if ((temp[index] == 'j') && (temp[index + 1] == 's'))
	{
		char str1[] = {"text/javascript"}
		memcpy(MIME_string, str1, sizeof(str1));
		return JS;
	}
	else if ((temp[index] == 'j') && (temp[index + 1] == 's') && (temp[index + 2] == 'o') && (temp[index + 3] == 'n') && (temp[index + 4] != 'l'))
	{
		char str1[] = {"application/json"}
		memcpy(MIME_string, str1, sizeof(str1));
		return JSON;
	}
	else if ((temp[index] == 'm') && (temp[index + 1] == 'p') && (temp[index + 2] == '3'))
	{
		char str1[] = {"audio/mpeg"}
		memcpy(MIME_string, str1, sizeof(str1));
		return MP3;
	}
	else if ((temp[index] == 'p') && (temp[index + 1] == 'n') && (temp[index + 2] == 'g'))
	{
		char str1[] = {"image/png"}
		memcpy(MIME_string, str1, sizeof(str1));
		return PNG;
	}
	else if ((temp[index] == 't') && (temp[index + 1] == 'i') && (temp[index + 2] == 'f'))
	{
		char str1[] = {"image/tiff"}
		memcpy(MIME_string, str1, sizeof(str1));
		return TIF;
	}
	else if ((temp[index] == 't') && (temp[index + 1] == 'x') && (temp[index + 2] == 't'))
	{
		char str1[] = {"text/plain"}
		memcpy(MIME_string, str1, sizeof(str1));
		return TXT;
	}
	else if ((temp[index] == 'w') && (temp[index + 1] == 'a') && (temp[index + 2] == 'v'))
	{
		char str1[] = {"audio/wav"}
		memcpy(MIME_string, str1, sizeof(str1));
		return WAV;
	}
	if ((temp[index] == 'x') && (temp[index + 1] == 'h') && (temp[index + 2] == 't'))
	{
		char str1[] = {"application/xhtml+xml"}
		memcpy(MIME_string, str1, sizeof(str1));
		return XHTML;
	}
	else if ((temp[index] == 'x') && (temp[index + 1] == 'm') && (temp[index + 2] == 'l'))
	{
		char str1[] = {"application/xml"}
		memcpy(MIME_string, str1, sizeof(str1));
		return XML;
	}
	else
	{
		char str1[] = {""}
		memcpy(MIME_string, str1, sizeof(str1));
		return UNKNOWN;
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
	
	uint32_t bodySize = sizeof(body);
	memccpy(&sendMsg[index], header, sizeof(header))
	index += sizeof(header);
	snprintf(len, sizeof(len), "%d", bodySize);
	memcpy(&sendMsg[index], len, sizeof(len));
	index += sizeof(len);
	memcpy(&sendMsg[index], "\n\n", 2);
	index += 2;
	memcpy(&sendMsg[index], temp, bodySize);
}

//*****************************************************************************
// main
//*****************************************************************************
int main(void)
{
	char sendBuffer[30000] = {0};

	int server_fd = socket(domain, type, protocol);
	if ( server_fd == 0 )
	{
		//perror("Cannot Create Socket");
		return (0);
	}
	memset(address.sin_zero, 0, sizeof(address.sin_zero));
	address.sin_family	= domain;
	address.sin_port   	= htons(myPortNumber);
	address.sin_addr.s_addr	= htonl(INADDR_ANY);
	int retVal = bind(server_fd, (struct sockaddr *)&address, sizeof(address));
	if ( retVal < 0 )
	{
		//perror("bind Failed");
		return 0;  
	}
	retVal = listen( server_fd, maxConnections );
	if ( retVal < 0 )
	{
		//perror("Listen Failure");
		exit(EXIT_FAILURE);
	}
	while(true)  //(exit == false)
	{
		newSocket = accept( server_fd, (struct sockaddr *)&address, (socklen_t*)&address_len);
		if ( newSocket < 0 )
		{
			//perror("Accept Failure");
			exit(EXIT_FAILURE); 
		}
		retVal = read( newSocket, buffer, BUFFER_SIZE );
		//printf("%s\n, buffer);
		if ( retVal < 0 )
		{
			//printf("No Bytes are there to read");
		}
		METHODS clientMethod = GetClientMethod(buffer);
		if(clientMethod == UNKNOWN)
		{
			close(newSocket);
			continue;
		}
		char requestedFile[100];
		if(GetFileName(buffer, requestedFile) == FALSE)
		{
			close(newSocket);
			continue;
		}
		char responseMIME[30];
		MIME clientMIME = GetMIMEType(buffer, responseMIME);
		if ( clientMIME == UNKNOWN )
		{
			close(newSocket)
			continue;
		}
		
		if ( clientMethod == GET )
		{
			if(responseMIME == TXT)
			{
				AssembleResponse(indexPage, sendBuffer);
			}
		}
		else if ( clientMethod == POST )
		{
			
		}
		else
		{
			close(newSocket);
			continue;
		}
		
		write(newSocket, sendBuffer, strlen(sendBuffer));
		//write(newSocket, header, strlen(header));
		close(newSocket);
	}
	return (0);
}

