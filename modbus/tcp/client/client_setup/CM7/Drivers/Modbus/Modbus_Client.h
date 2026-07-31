/*
 * Modbus_Client.h
 *
 *  Created on: 27-Jul-2026
 *      Author: arunrawat
 */


#ifndef MODBUS_CLIENT_H_
#define MODBUS_CLIENT_H_

#include "lwip/tcp.h"

err_t TCP_SendRequest(struct tcp_pcb *tpcb, uint8_t *txBuf, uint16_t length);
void Modbus_Client_Process(char *serverIP, uint16_t port);

#endif
