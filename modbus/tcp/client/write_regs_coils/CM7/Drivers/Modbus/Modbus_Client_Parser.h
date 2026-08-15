/*
 * Modbus_Client_Parser.h
 *
 *  Created on: 27-Jul-2026
 *      Author: arunrawat
 */


#ifndef MODBUS_CLIENT_PARSER_H_
#define MODBUS_CLIENT_PARSER_H_

#include "lwip/tcp.h"

err_t MB_Client_Parser(struct tcp_pcb*,uint8_t*,uint16_t);
err_t MB_ReadHoldingRegisters(uint16_t txnID, uint8_t unitID, uint16_t addr, uint16_t qty);

#endif
