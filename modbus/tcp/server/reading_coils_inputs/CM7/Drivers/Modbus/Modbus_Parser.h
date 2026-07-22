/*
 * Modbus_Parser.h
 *
 *  Created on: 29-Jun-2026
 *      Author: arunrawat
 */

#ifndef MODBUS_MODBUS_PARSER_H_
#define MODBUS_MODBUS_PARSER_H_

#include "lwip/tcp.h"

err_t MB_Parser(struct tcp_pcb *pcb, uint8_t *rxBuf, uint16_t length);

#endif /* MODBUS_MODBUS_PARSER_H_ */
