/*
 * Modbus_Client_Parser.c
 *
 *  Created on: 27-Jul-2026
 *      Author: arunrawat
 */


#include "Modbus_Client_Parser.h"
#include "modbus.h"
#include "Modbus_Client.h"


err_t MB_Client_Parser(struct tcp_pcb *pcb,uint8_t *rx,uint16_t len)
{
    printf("Received %d bytes\r\n", len);
	return ERR_OK;
}


