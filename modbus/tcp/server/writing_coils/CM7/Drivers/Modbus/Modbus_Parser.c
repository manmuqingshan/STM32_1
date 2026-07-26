/*
 * Modbus_Parser.c
 *
 *  Created on: 29-Jun-2026
 *      Author: arunrawat
 */


#include "Modbus_Parser.h"
#include "modbus.h"
#include "Modbus_Database.h"
#include "lwip/tcp.h"
#include "lwip/pbuf.h"

#include <string.h>

static err_t MB_ParseRequest(uint8_t *rxBuf, uint16_t length, MB_Request_t *req);
static err_t MB_ProcessFunction(struct tcp_pcb *pcb, MB_Request_t *req);
static err_t MB_SendException(struct tcp_pcb *pcb, MB_Request_t *req, uint8_t exception);
static err_t MB_FC03_ReadHoldingRegisters(struct tcp_pcb *pcb, MB_Request_t *req);
static err_t MB_FC04_ReadInputRegisters(struct tcp_pcb *pcb, MB_Request_t *req);
static err_t MB_FC06_WriteSingleRegister(struct tcp_pcb *pcb, MB_Request_t *req);
static err_t MB_FC16_WriteMultipleRegisters(struct tcp_pcb *pcb, MB_Request_t *req);
static err_t MB_FC01_ReadCoils(struct tcp_pcb *pcb, MB_Request_t *req);
static err_t MB_FC02_ReadDisInputs(struct tcp_pcb *pcb, MB_Request_t *req);
static err_t MB_FC05_WriteSingleCoil(struct tcp_pcb *pcb, MB_Request_t *req);
static err_t MB_FC15_WriteMultipleCoils(struct tcp_pcb *pcb, MB_Request_t *req);

static void MB_PrintHex(const uint8_t *buf, uint16_t len);

err_t MB_Parser(struct tcp_pcb *pcb, uint8_t *rxBuf, uint16_t length)
{
    MB_Request_t req;

    printf("\r\n========== Modbus TCP Request ==========\r\n");
    MB_PrintHex(rxBuf, length);

    /* Parse incoming request */
    if(MB_ParseRequest(rxBuf, length, &req) != ERR_OK)
    {
        printf("Request Parsing Failed!\r\n");
        return ERR_VAL;
    }

    printf("Request Parsed Successfully\r\n");

    /* Execute requested function */
    return MB_ProcessFunction(pcb, &req);
}



static err_t MB_ParseRequest(uint8_t *rxBuf, uint16_t length, MB_Request_t *req)
{
    if(length < 8)
    {
        printf("Error: Packet too short (%d bytes)\r\n", length);
        return ERR_VAL;
    }

    req->TransactionID = ((uint16_t)rxBuf[0] << 8) | rxBuf[1];
    req->ProtocolID    = ((uint16_t)rxBuf[2] << 8) | rxBuf[3];
    req->Length        = ((uint16_t)rxBuf[4] << 8) | rxBuf[5];
    req->UnitID        = rxBuf[6];
    req->FunctionCode  = rxBuf[7];

    req->Data = &rxBuf[8];
    req->DataLength = length - 8;

    printf("Transaction ID : %u\r\n", req->TransactionID);
    printf("Protocol ID    : %u\r\n", req->ProtocolID);
    printf("Length         : %u\r\n", req->Length);
    printf("Unit ID        : %u\r\n", req->UnitID);
    printf("Function Code  : 0x%02X\r\n", req->FunctionCode);
    printf("Data Length    : %u\r\n", req->DataLength);

    if(req->ProtocolID != 0)
    {
        printf("Error: Invalid Protocol ID\r\n");
        return ERR_VAL;
    }

    return ERR_OK;
}

static err_t MB_ProcessFunction(struct tcp_pcb *pcb, MB_Request_t *req)
{
    printf("Processing Function Code : 0x%02X\r\n", req->FunctionCode);

    switch(req->FunctionCode)
    {
    case MB_FC_READ_HOLDING_REGS:
    	return MB_FC03_ReadHoldingRegisters(pcb, req);

    case MB_FC_READ_INPUT_REGS:
    	return MB_FC04_ReadInputRegisters(pcb, req);

    case MB_FC_WRITE_SINGLE_REG:
        return MB_FC06_WriteSingleRegister(pcb, req);

    case MB_FC_WRITE_MULTI_REGS:
        return MB_FC16_WriteMultipleRegisters(pcb, req);

    case MB_FC_READ_COILS:
        return MB_FC01_ReadCoils(pcb, req);

    case MB_FC_READ_DISCRETE_INPUTS:
        return MB_FC02_ReadDisInputs(pcb, req);

    case MB_FC_WRITE_SINGLE_COIL:
        return MB_FC05_WriteSingleCoil(pcb, req);

    case MB_FC_WRITE_MULTI_COILS:
        return MB_FC15_WriteMultipleCoils(pcb, req);

    default:
        printf("Unsupported Function Code\r\n");
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_FUNCTION);
    }
}

static err_t MB_SendException(struct tcp_pcb *pcb, MB_Request_t *req, uint8_t exception)
{
    uint8_t tx[9];

    /* MBAP Header */
    tx[0] = req->TransactionID >> 8;
    tx[1] = req->TransactionID;

    tx[2] = 0;
    tx[3] = 0;

    tx[4] = 0;
    tx[5] = 3;

    tx[6] = req->UnitID;

    /* PDU */
    tx[7] = req->FunctionCode | 0x80;

    tx[8] = exception;

    printf("Sending Exception Response\r\n");
    printf("Exception Code : 0x%02X\r\n", exception);

    if(tcp_write(pcb, tx, sizeof(tx), TCP_WRITE_FLAG_COPY) != ERR_OK)
    {
        printf("tcp_write() Failed\r\n");
        return ERR_MEM;
    }

    tcp_output(pcb);

    printf("Response Sent Successfully\r\n");

    return ERR_OK;
}


static err_t MB_FC03_ReadHoldingRegisters(struct tcp_pcb *pcb, MB_Request_t *req)
{
    uint16_t startAddr;
    uint16_t quantity;
    uint16_t i;
    uint8_t tx[260];
    uint16_t txLen = 0;

    startAddr = ((uint16_t)req->Data[0] << 8) | req->Data[1];
    quantity  = ((uint16_t)req->Data[2] << 8) | req->Data[3];

    printf("FC03 - Read Holding Registers\r\n");
    printf("Start Address : %u\r\n", startAddr);
    printf("Quantity      : %u\r\n", quantity);

    /* Validate quantity */
    if((quantity == 0) || (quantity > 125))
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_VALUE);

    /* Validate address range */
    if((startAddr + quantity) > MB_HOLDING_REG_COUNT)
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_ADDRESS);

    /* MBAP Header */
    // Transaction ID 2 Bytes
    tx[0] = req->TransactionID >> 8;
    tx[1] = req->TransactionID;

    // Fixed Protocol ID 0000
    tx[2] = 0;
    tx[3] = 0;

    // Length 2 Bytes
    uint16_t length = 3 + (quantity * 2);  // Unit ID + FC + NumBytes + 2 bytes per reg
    tx[4] = length >> 8;
    tx[5] = length;

    tx[6] = req->UnitID;  // Unit ID

    /* PDU */
    tx[7] = MB_FC_READ_HOLDING_REGS;  // FC
    tx[8] = quantity * 2;  // Num Bytes (Data)

    txLen = 9;

    for(i = 0; i < quantity; i++)
    {
        uint16_t value = MB_HoldingRegs[startAddr + i];

        tx[txLen++] = value >> 8;
        tx[txLen++] = value;
    }

    if(tcp_write(pcb, tx, txLen, TCP_WRITE_FLAG_COPY) != ERR_OK)
        return ERR_MEM;

    tcp_output(pcb);

    printf("FC03 Response Sent\r\n");

    return ERR_OK;
}

static err_t MB_FC04_ReadInputRegisters(struct tcp_pcb *pcb, MB_Request_t *req)
{
    uint16_t startAddr;
    uint16_t quantity;
    uint16_t i;
    uint8_t tx[260];
    uint16_t txLen = 0;

    startAddr = ((uint16_t)req->Data[0] << 8) | req->Data[1];
    quantity  = ((uint16_t)req->Data[2] << 8) | req->Data[3];

    printf("FC04 - Read Input Registers\r\n");
    printf("Start Address : %u\r\n", startAddr);
    printf("Quantity      : %u\r\n", quantity);

    /* Validate quantity */
    if((quantity == 0) || (quantity > 125))
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_VALUE);

    /* Validate address range */
    if((startAddr + quantity) > MB_INPUT_REG_COUNT)
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_ADDRESS);

    /* MBAP Header */
    // Transaction ID 2 Bytes
    tx[0] = req->TransactionID >> 8;
    tx[1] = req->TransactionID;

    // Fixed Protocol ID 0000
    tx[2] = 0;
    tx[3] = 0;

    // Length 2 Bytes
    uint16_t length = 3 + (quantity * 2);  // Unit ID + FC + NumBytes + 2 bytes per reg
    tx[4] = length >> 8;
    tx[5] = length;

    tx[6] = req->UnitID;  // Unit ID

    /* PDU */
    tx[7] = MB_FC_READ_INPUT_REGS;  // FC
    tx[8] = quantity * 2;  // Num Bytes (Data)

    txLen = 9;

    for(i = 0; i < quantity; i++)
    {
        uint16_t value = MB_InputRegs[startAddr + i];

        tx[txLen++] = value >> 8;
        tx[txLen++] = value;
    }

    if(tcp_write(pcb, tx, txLen, TCP_WRITE_FLAG_COPY) != ERR_OK)
        return ERR_MEM;

    tcp_output(pcb);

    printf("FC04 Response Sent\r\n");

    return ERR_OK;
}


static err_t MB_FC06_WriteSingleRegister(struct tcp_pcb *pcb, MB_Request_t *req)
{
    uint16_t address;
    uint16_t value;
    uint8_t tx[12];

    address = ((uint16_t)req->Data[0] << 8) | req->Data[1];
    value   = ((uint16_t)req->Data[2] << 8) | req->Data[3];

    printf("FC06 - Write Single Holding Register\r\n");
    printf("Address : %u\r\n", address);
    printf("Value   : %u (0x%04X)\r\n", value, value);

    /* Validate address */
    if(address >= MB_HOLDING_REG_COUNT)
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_ADDRESS);

    /* Write Register */
    MB_HoldingRegs[address] = value;

    /* MBAP Header */
    tx[0] = req->TransactionID >> 8;
    tx[1] = req->TransactionID;

    tx[2] = 0;
    tx[3] = 0;

    tx[4] = 0;
    tx[5] = 6;      // Unit ID + FC + Address + Value

    tx[6] = req->UnitID;

    /* PDU */
    tx[7] = MB_FC_WRITE_SINGLE_REG;

    tx[8]  = address >> 8;
    tx[9]  = address;

    tx[10] = value >> 8;
    tx[11] = value;

    if(tcp_write(pcb, tx, sizeof(tx), TCP_WRITE_FLAG_COPY) != ERR_OK)
        return ERR_MEM;

    tcp_output(pcb);

    printf("FC06 Response Sent\r\n");

    return ERR_OK;
}

static err_t MB_FC16_WriteMultipleRegisters(struct tcp_pcb *pcb, MB_Request_t *req)
{
    uint16_t startAddr;
    uint16_t quantity;
    uint8_t byteCount;
    uint16_t value;
    uint16_t i;

    uint8_t tx[12];

    startAddr = ((uint16_t)req->Data[0] << 8) | req->Data[1];
    quantity  = ((uint16_t)req->Data[2] << 8) | req->Data[3];
    byteCount = req->Data[4];

    printf("FC16 - Write Multiple Holding Registers\r\n");
    printf("Start Address : %u\r\n", startAddr);
    printf("Quantity      : %u\r\n", quantity);
    printf("Byte Count    : %u\r\n", byteCount);

    /* Validate quantity */
    if((quantity == 0) || (quantity > 123))
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_VALUE);

    /* Validate address range */
    if((startAddr + quantity) > MB_HOLDING_REG_COUNT)
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_ADDRESS);

    /* Write Registers */
    for(i = 0; i < quantity; i++)
    {
        value = ((uint16_t)req->Data[5 + (i * 2)] << 8) | req->Data[6 + (i * 2)];

        MB_HoldingRegs[startAddr + i] = value;

        printf("Reg[%u] = %u (0x%04X)\r\n", startAddr + i, value, value);
    }

    /* MBAP Header */
    tx[0] = req->TransactionID >> 8;
    tx[1] = req->TransactionID;

    tx[2] = 0;
    tx[3] = 0;

    tx[4] = 0;
    tx[5] = 6;      // Unit ID + FC + Address + Quantity

    tx[6] = req->UnitID;

    /* PDU */
    tx[7] = MB_FC_WRITE_MULTI_REGS;

    tx[8] = startAddr >> 8;
    tx[9] = startAddr;

    tx[10] = quantity >> 8;
    tx[11] = quantity;

    if(tcp_write(pcb, tx, sizeof(tx), TCP_WRITE_FLAG_COPY) != ERR_OK)
        return ERR_MEM;

    tcp_output(pcb);

    printf("FC16 Response Sent\r\n");

    return ERR_OK;
}


static err_t MB_FC01_ReadCoils(struct tcp_pcb *pcb, MB_Request_t *req)
{
	uint16_t startAddr;
	uint16_t quantity;
	uint16_t i;
	uint8_t tx[260];
	uint16_t txLen = 0;

	startAddr = ((uint16_t)req->Data[0] << 8) | req->Data[1];
	quantity  = ((uint16_t)req->Data[2] << 8) | req->Data[3];

	uint8_t byteCount = (quantity + 7) / 8;  // 8 coils per Byte

	printf("FC01 - Read Coils\r\n");
	printf("Start Address : %u\r\n", startAddr);
	printf("Quantity      : %u\r\n", quantity);

	/* Validate quantity */
	if((quantity == 0) || (quantity > 2000))
		return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_VALUE);

	/* Validate address range */
	if((startAddr + quantity) > MB_COIL_COUNT)
		return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_ADDRESS);

	/* MBAP Header */
	// Transaction ID 2 Bytes
	tx[0] = req->TransactionID >> 8;
	tx[1] = req->TransactionID;

	// Fixed Protocol ID 0000
	tx[2] = 0;
	tx[3] = 0;

	// Length 2 Bytes
	uint16_t length = 3 + byteCount;  // Unit ID + FC + NumBytes + actual data
	tx[4] = length >> 8;
	tx[5] = length;

	tx[6] = req->UnitID;  // Unit ID

	/* PDU */
	tx[7] = MB_FC_READ_COILS;  // FC
	tx[8] = byteCount;  // Num Bytes (Data)

	txLen = 9;
	memset(&tx[9], 0, byteCount);

	// start copying data
	for(i = 0; i < quantity; i++)
	{
	    uint16_t coilAddress = startAddr + i;  // current coil Number

	    uint16_t sourceByte = coilAddress / 8;  // Which byte contains this coil
	    uint8_t sourceBit   = coilAddress % 8;  // Which bit inside that byte

	    uint16_t destByte = i / 8;  // Which response byte should receive this coil
	    uint8_t destBit   = i % 8;  // Which bit inside the response byte

	    if((MB_Coils[sourceByte] >> sourceBit) & 0x01)  // If the coil bit is set
	    {
	        tx[9 + destByte] |= (1 << destBit);  // set the bit in the TX Buffer or leave it (Reset)
	    }
	}

	txLen = 9 + byteCount;

	if(tcp_write(pcb, tx, txLen, TCP_WRITE_FLAG_COPY) != ERR_OK)
		return ERR_MEM;

	tcp_output(pcb);

	printf("FC01 Response Sent\r\n");

	return ERR_OK;
}


static err_t MB_FC02_ReadDisInputs(struct tcp_pcb *pcb, MB_Request_t *req)
{
	uint16_t startAddr;
	uint16_t quantity;
	uint16_t i;
	uint8_t tx[260];
	uint16_t txLen = 0;

	startAddr = ((uint16_t)req->Data[0] << 8) | req->Data[1];
	quantity  = ((uint16_t)req->Data[2] << 8) | req->Data[3];

	uint8_t byteCount = (quantity + 7) / 8;  // 8 Inputs per Byte

	printf("FC02 - Read Discrete Inputs\r\n");
	printf("Start Address : %u\r\n", startAddr);
	printf("Quantity      : %u\r\n", quantity);

	/* Validate quantity */
	if((quantity == 0) || (quantity > 2000))
		return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_VALUE);

	/* Validate address range */
	if((startAddr + quantity) > MB_DIS_INPUT_COUNT)
		return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_ADDRESS);

	/* MBAP Header */
	// Transaction ID 2 Bytes
	tx[0] = req->TransactionID >> 8;
	tx[1] = req->TransactionID;

	// Fixed Protocol ID 0000
	tx[2] = 0;
	tx[3] = 0;

	// Length 2 Bytes
	uint16_t length = 3 + byteCount;  // Unit ID + FC + NumBytes + actual data
	tx[4] = length >> 8;
	tx[5] = length;

	tx[6] = req->UnitID;  // Unit ID

	/* PDU */
	tx[7] = MB_FC_READ_DISCRETE_INPUTS;  // FC
	tx[8] = byteCount;  // Num Bytes (Data)

	txLen = 9;
	memset(&tx[9], 0, byteCount);

	// start copying data
	for(i = 0; i < quantity; i++)
	{
	    uint16_t coilAddress = startAddr + i;  // current coil Number

	    uint16_t sourceByte = coilAddress / 8;  // Which byte contains this coil
	    uint8_t sourceBit   = coilAddress % 8;  // Which bit inside that byte

	    uint16_t destByte = i / 8;  // Which response byte should receive this coil
	    uint8_t destBit   = i % 8;  // Which bit inside the response byte

	    if((MB_DisInputs[sourceByte] >> sourceBit) & 0x01)  // If the coil bit is set
	    {
	        tx[9 + destByte] |= (1 << destBit);  // set the bit in the TX Buffer or leave it (Reset)
	    }
	}

	txLen = 9 + byteCount;

	if(tcp_write(pcb, tx, txLen, TCP_WRITE_FLAG_COPY) != ERR_OK)
		return ERR_MEM;

	tcp_output(pcb);

	printf("FC02 Response Sent\r\n");

	return ERR_OK;
}


static err_t MB_FC05_WriteSingleCoil(struct tcp_pcb *pcb, MB_Request_t *req)
{
    uint16_t address;
    uint16_t value;
    uint8_t tx[12];

    address = ((uint16_t)req->Data[0] << 8) | req->Data[1];
    value   = ((uint16_t)req->Data[2] << 8) | req->Data[3];

    printf("FC05 - Write Single Coil\r\n");
    printf("Address : %u\r\n", address);
    printf("Value   : %u (0x%04X)\r\n", value, value);

    /* Validate address */
    if(address >= MB_COIL_COUNT)
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_ADDRESS);

    /* Write Register */
   	uint16_t sourceByte = address / 8;  // Which byte contains this coil
   	uint8_t sourceBit   = address % 8;  // Which bit inside that byte

   	if(value == 0xFF00)
   	{
   	    MB_Coils[sourceByte] |= (1 << sourceBit);
   	}
   	else if(value == 0x0000)
   	{
   	    MB_Coils[sourceByte] &= ~(1 << sourceBit);
   	}
   	else
   	{
   	    return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_VALUE);
   	}

    /* MBAP Header */
    tx[0] = req->TransactionID >> 8;
    tx[1] = req->TransactionID;

    tx[2] = 0;
    tx[3] = 0;

    tx[4] = 0;
    tx[5] = 6;      // Unit ID + FC + Address + Value

    tx[6] = req->UnitID;

    /* PDU */
    tx[7] = MB_FC_WRITE_SINGLE_COIL;

    tx[8]  = address >> 8;
    tx[9]  = address;

    tx[10] = value >> 8;
    tx[11] = value;

    if(tcp_write(pcb, tx, sizeof(tx), TCP_WRITE_FLAG_COPY) != ERR_OK)
        return ERR_MEM;

    tcp_output(pcb);

    printf("FC05 Response Sent\r\n");

    return ERR_OK;
}


static err_t MB_FC15_WriteMultipleCoils(struct tcp_pcb *pcb, MB_Request_t *req)
{
    uint16_t startAddr;
    uint16_t quantity;
    uint8_t byteCount;
    uint8_t expectedByteCount;
    uint16_t i;

    uint8_t tx[12];

    startAddr = ((uint16_t)req->Data[0] << 8) | req->Data[1];
    quantity  = ((uint16_t)req->Data[2] << 8) | req->Data[3];
    byteCount = req->Data[4];

    printf("FC15 - Write Multiple Coils\r\n");
    printf("Start Address : %u\r\n", startAddr);
    printf("Quantity      : %u\r\n", quantity);
    printf("Byte Count    : %u\r\n", byteCount);

    /* Validate quantity */
    if((quantity == 0) || (quantity > 1968))
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_VALUE);

    /* Validate byte count */
    expectedByteCount = (quantity + 7) / 8;
    if(byteCount != expectedByteCount)
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_VALUE);

    /* Validate address range */
    if((startAddr + quantity) > MB_COIL_COUNT)
        return MB_SendException(pcb, req, MB_EX_ILLEGAL_DATA_ADDRESS);

    /* Write each requested coil */
    for(i = 0; i < quantity; i++)
    {
        uint8_t sourceByte = i / 8;          // Byte in the request data containing the current coil
        uint8_t sourceBit  = i % 8;          // Bit position within that byte

        uint8_t state = (req->Data[5 + sourceByte] >> sourceBit) & 0x01;   // Extract the coil state (0 or 1)

        uint16_t coilAddress = startAddr + i;    // Actual coil address in the server database

        uint16_t coilByte = coilAddress / 8;     // Byte in the coil database containing this coil
        uint8_t coilBit = coilAddress % 8;       // Bit position within that byte

        if(state)
            MB_Coils[coilByte] |= (1 << coilBit);     // Set the coil
        else
            MB_Coils[coilByte] &= ~(1 << coilBit);    // Clear the coil

        printf("Coil[%u] = %u\r\n", coilAddress, state);
    }

    /* MBAP Header */
    tx[0] = req->TransactionID >> 8;
    tx[1] = req->TransactionID;

    tx[2] = 0;
    tx[3] = 0;

    tx[4] = 0;
    tx[5] = 6;      // Unit ID + FC + Address + Quantity

    tx[6] = req->UnitID;

    /* PDU */
    tx[7] = MB_FC_WRITE_MULTI_COILS;

    tx[8] = startAddr >> 8;
    tx[9] = startAddr;

    tx[10] = quantity >> 8;
    tx[11] = quantity;

    if(tcp_write(pcb, tx, sizeof(tx), TCP_WRITE_FLAG_COPY) != ERR_OK)
        return ERR_MEM;

    tcp_output(pcb);

    printf("FC15 Response Sent\r\n");

    return ERR_OK;
}


static void MB_PrintHex(const uint8_t *buf, uint16_t len)
{
    printf("RX Data (%d Bytes):\r\n", len);

    for(uint16_t i = 0; i < len; i++)
    {
        printf("%02X ", buf[i]);

        if((i + 1) % 16 == 0)
            printf("\r\n");
    }

    printf("\r\n");
}



