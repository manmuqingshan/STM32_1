/*
 * modbus.h
 *
 *  Created on: 29-Jun-2026
 *      Author: arunrawat
 */

#include "main.h"

#ifndef MODBUS_MODBUS_H_
#define MODBUS_MODBUS_H_

#define MB_TCP_PORT                     502

#define MB_FC_READ_COILS            0x01
#define MB_FC_READ_DISCRETE_INPUTS  0x02
#define MB_FC_READ_HOLDING_REGS     0x03
#define MB_FC_READ_INPUT_REGS       0x04
#define MB_FC_WRITE_SINGLE_COIL     0x05
#define MB_FC_WRITE_SINGLE_REG      0x06
#define MB_FC_WRITE_MULTI_COILS     0x0F
#define MB_FC_WRITE_MULTI_REGS      0x10



#define MB_EX_ILLEGAL_FUNCTION        0x01
#define MB_EX_ILLEGAL_DATA_ADDRESS    0x02
#define MB_EX_ILLEGAL_DATA_VALUE      0x03
#define MB_EX_SERVER_DEVICE_FAILURE   0x04


typedef struct
{
    uint16_t TransactionID;
    uint16_t ProtocolID;
    uint16_t Length;
    uint8_t  UnitID;

    uint8_t  FunctionCode;

    uint8_t *Data;
    uint16_t DataLength;

} MB_Request_t;

#endif /* MODBUS_MODBUS_H_ */
