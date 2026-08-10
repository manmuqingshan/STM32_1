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
    tx[1]=txnID++;

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
    tx[1]=txnID++;

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
    tx[1] = txnID++;

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
    tx[1] = txnID++;

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
