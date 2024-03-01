/*
 * Server.h
 *
 *  Created on: Jul 20, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_NETWORK_SERVER_H_
#define INC_NETWORK_SERVER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "Globals.h"
#include "tcp.h"
#include "err.h"
#include "ip_addr.h"
#include "pbuf.h"
#include "udp.h"
#include <sockets.h>

BOOL ServerInit(void);
void ServerTask(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_NETWORK_SERVER_H_ */

//lines 3
