/*
 * Modbus_Client_Request.c
 *
 *  Created on: 27-Jul-2026
 *      Author: arunrawat
 */


#include "modbus.h"
#include "Modbus_Client_Request.h"
#include "Modbus_Client.h"

MB_Request_t MB_CurrentRequest;

err_t MB_ReadHoldingRegisters(uint16_t txnID, uint8_t unitID, uint16_t addr, uint16_t qty)
{
	MB_CurrentRequest.StartAddress = addr;
	MB_CurrentRequest.Quantity = qty;

    uint8_t tx[12];
    /* MBAP Header */
    // Transaction ID
    tx[0]=txnID>>8;
    tx[1]=txnID;

    // Protocol ID
    tx[2]=0;
    tx[3]=0;

    //Length
    tx[4]=0;
    tx[5]=6;

    // Unit ID
    tx[6]=unitID;

    /* PDU */
    // Function Code
    tx[7]=MB_FC_READ_HOLDING_REGS;

    // Start Address
    tx[8]=addr>>8;
    tx[9]=addr;

    // Quantity
    tx[10]=qty>>8;
    tx[11]=qty;

    /* Send the Request */
    return TCP_SendRequest(tx, 12);
}


err_t MB_ReadInputRegisters(uint16_t txnID, uint8_t unitID, uint16_t addr, uint16_t qty)
{
	MB_CurrentRequest.StartAddress = addr;
	MB_CurrentRequest.Quantity = qty;

    uint8_t tx[12];
    /* MBAP Header */
    // Transaction ID
    tx[0]=txnID>>8;
    tx[1]=txnID;

    // Protocol ID
    tx[2]=0;
    tx[3]=0;

    //Length
    tx[4]=0;
    tx[5]=6;

    // Unit ID
    tx[6]=unitID;

    /* PDU */
    // Function Code
    tx[7]=MB_FC_READ_INPUT_REGS;

    // Start Address
    tx[8]=addr>>8;
    tx[9]=addr;

    // Quantity
    tx[10]=qty>>8;
    tx[11]=qty;

    /* Send the Request */
    return TCP_SendRequest(tx, 12);
}

err_t MB_ReadCoils(uint16_t txnID, uint8_t unitID, uint16_t addr, uint16_t qty)
{
    MB_CurrentRequest.StartAddress = addr;
    MB_CurrentRequest.Quantity = qty;

    uint8_t tx[12];

    /* MBAP Header */

    // Transaction ID
    tx[0] = txnID >> 8;
    tx[1] = txnID;

    // Protocol ID
    tx[2] = 0;
    tx[3] = 0;

    // Length
    tx[4] = 0;
    tx[5] = 6;

    // Unit ID
    tx[6] = unitID;

    /* PDU */

    // Function Code
    tx[7] = MB_FC_READ_COILS;

    // Start Address
    tx[8] = addr >> 8;
    tx[9] = addr;

    // Quantity
    tx[10] = qty >> 8;
    tx[11] = qty;

    /* Send the Request */
    return TCP_SendRequest(tx, 12);
}

err_t MB_ReadDiscreteInputs(uint16_t txnID, uint8_t unitID, uint16_t addr, uint16_t qty)
{
    MB_CurrentRequest.StartAddress = addr;
    MB_CurrentRequest.Quantity = qty;

    uint8_t tx[12];

    /* MBAP Header */

    // Transaction ID
    tx[0] = txnID >> 8;
    tx[1] = txnID;

    // Protocol ID
    tx[2] = 0;
    tx[3] = 0;

    // Length
    tx[4] = 0;
    tx[5] = 6;

    // Unit ID
    tx[6] = unitID;

    /* PDU */

    // Function Code
    tx[7] = MB_FC_READ_DISCRETE_INPUTS;

    // Start Address
    tx[8] = addr >> 8;
    tx[9] = addr;

    // Quantity
    tx[10] = qty >> 8;
    tx[11] = qty;

    /* Send the Request */
    return TCP_SendRequest(tx, 12);
}

err_t MB_WriteSingleRegister(uint16_t txnID, uint8_t unitID, uint16_t addr, uint16_t value)
{
    MB_CurrentRequest.StartAddress = addr;

    uint8_t tx[12];

    /* MBAP Header */

    // Transaction ID
    tx[0] = txnID >> 8;
    tx[1] = txnID;

    // Protocol ID
    tx[2] = 0;
    tx[3] = 0;

    // Length
    tx[4] = 0;
    tx[5] = 6;

    // Unit ID
    tx[6] = unitID;

    /* PDU */

    // Function Code
    tx[7] = MB_FC_WRITE_SINGLE_REG;

    // Register Address
    tx[8] = addr >> 8;
    tx[9] = addr;

    // Register Value
    tx[10] = value >> 8;
    tx[11] = value;

    /* Send the Request */
    return TCP_SendRequest(tx, 12);
}


err_t MB_WriteMultipleRegisters(uint16_t txnID, uint8_t unitID, uint16_t addr, uint16_t qty, uint16_t *data)
{
    MB_CurrentRequest.StartAddress = addr;
    MB_CurrentRequest.Quantity = qty;

    uint8_t tx[260];
    uint8_t byteCount = qty * 2;

    /* MBAP Header */

    // Transaction ID
    tx[0] = txnID >> 8;
    tx[1] = txnID;

    // Protocol ID
    tx[2] = 0;
    tx[3] = 0;

    // Length
    tx[4] = (7 + byteCount) >> 8;
    tx[5] = (7 + byteCount);

    // Unit ID
    tx[6] = unitID;

    /* PDU */

    // Function Code
    tx[7] = MB_FC_WRITE_MULTI_REGS;

    // Starting Address
    tx[8] = addr >> 8;
    tx[9] = addr;

    // Quantity
    tx[10] = qty >> 8;
    tx[11] = qty;

    // Byte Count
    tx[12] = byteCount;

    /* Register Data */

    for(uint16_t i = 0; i < qty; i++)
    {
        tx[13 + (i * 2)] = data[i] >> 8;
        tx[14 + (i * 2)] = data[i];
    }

    /* Send the Request */
    return TCP_SendRequest(tx, 13 + byteCount);
}


err_t MB_WriteSingleCoil(uint16_t txnID, uint8_t unitID, uint16_t addr, uint8_t value)
{
    MB_CurrentRequest.StartAddress = addr;

    uint8_t tx[12];

    /* MBAP Header */

    // Transaction ID
    tx[0] = txnID >> 8;
    tx[1] = txnID;

    // Protocol ID
    tx[2] = 0;
    tx[3] = 0;

    // Length
    tx[4] = 0;
    tx[5] = 6;

    // Unit ID
    tx[6] = unitID;

    /* PDU */

    // Function Code
    tx[7] = MB_FC_WRITE_SINGLE_COIL;

    // Output Address
    tx[8] = addr >> 8;
    tx[9] = addr;

    // Output Value
    if(value)
    {
        tx[10] = 0xFF;
        tx[11] = 0x00;
    }
    else
    {
        tx[10] = 0x00;
        tx[11] = 0x00;
    }

    /* Send the Request */
    return TCP_SendRequest(tx, 12);
}


err_t MB_WriteMultipleCoils(uint16_t txnID, uint8_t unitID, uint16_t addr, uint16_t qty, uint8_t *data)  // data[0,1,1,0,0,1...]
{
    MB_CurrentRequest.StartAddress = addr;
    MB_CurrentRequest.Quantity = qty;

    uint8_t tx[260];
    uint8_t byteCount = (qty + 7) / 8;

    /* MBAP Header */

    // Transaction ID
    tx[0] = txnID >> 8;
    tx[1] = txnID;

    // Protocol ID
    tx[2] = 0;
    tx[3] = 0;

    // Length
    tx[4] = 0;
    tx[5] = 7 + byteCount;

    // Unit ID
    tx[6] = unitID;

    /* PDU */

    // Function Code
    tx[7] = MB_FC_WRITE_MULTI_COILS;

    // Starting Address
    tx[8] = addr >> 8;
    tx[9] = addr;

    // Quantity
    tx[10] = qty >> 8;
    tx[11] = qty;

    // Byte Count
    tx[12] = byteCount;

    /* Coil Data */

    for(uint8_t i = 0; i < byteCount; i++)
    {
        tx[13 + i] = 0;

        for(uint8_t bit = 0; bit < 8; bit++)
        {
            uint16_t index = (i * 8) + bit;

            if(index >= qty)
                break;

            if(data[index])
            {
                tx[13 + i] |= (1 << bit);
            }
        }
    }

    /* Send the Request */
    return TCP_SendRequest(tx, 13 + byteCount);
}


