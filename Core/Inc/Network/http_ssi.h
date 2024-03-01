/*
 * http_ssi.h
 *
 *  Created on: Dec 5, 2023
 *      Author: joe.dobesh
 */

#ifndef INC_NETWORK_HTTP_SSI_H_
#define INC_NETWORK_HTTP_SSI_H_

#ifdef __cplusplus
extern "C" {
#endif

void HttpdSsiInit(void);
uint16_t SsiHandler(int, char *, int);
void NetComInit(void);
void NetComTask(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_NETWORK_HTTP_SSI_H_ */

//EOF
