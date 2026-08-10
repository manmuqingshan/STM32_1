/*
 * Modbus_Client_Parser.c
 *
 *  Created on: 27-Jul-2026
 *      Author: arunrawat
 */


#include "Modbus_Client_Parser.h"
#include "modbus.h"
#include "Modbus_Client.h"
#include "Modbus_Client_Request.h"


static err_t MB_ParseReadCoils(uint8_t *rx);
static err_t MB_ParseReadDiscreteInputs(uint8_t *rx);
static err_t MB_ParseReadHoldingRegisters(uint8_t *rx);
static err_t MB_ParseReadInputRegisters(uint8_t *rx);
static err_t MB_ParseException(uint8_t *rx);

uint16_t HoldingRegBuffer[125];
uint16_t InputRegBuffer[125];
uint8_t CoilBuffer[2000];
uint8_t InputBuffer[2000];


/*-----------------------------------------------------------*/

err_t MB_Client_Parser(struct tcp_pcb *pcb, uint8_t *rx, uint16_t len)
{
    if(len < 9)
        return ERR_VAL;

    uint8_t FunctionCode = rx[7];

    /* Exception Response */
    if(FunctionCode & 0x80)
        return MB_ParseException(rx);

    switch(FunctionCode)
    {
        case MB_FC_READ_HOLDING_REGS:
            return MB_ParseReadHoldingRegisters(rx);

        case MB_FC_READ_INPUT_REGS:
            return MB_ParseReadInputRegisters(rx);

        case MB_FC_READ_COILS:
            return MB_ParseReadCoils(rx);

        case MB_FC_READ_DISCRETE_INPUTS:
            return MB_ParseReadDiscreteInputs(rx);

        default:
            printf("Unsupported Function Code : %d\r\n", FunctionCode);
            break;
    }

    return ERR_OK;
}

static err_t MB_ParseReadHoldingRegisters(uint8_t *rx)
{
    uint8_t byteCount = rx[8];
    uint16_t quantity = byteCount / 2;
    uint16_t startAddress = MB_CurrentRequest.StartAddress;

    /* Copy response into the buffer */
    for(uint16_t i = 0; i < quantity; i++)
    {
        HoldingRegBuffer[startAddress + i] = ((uint16_t)rx[9 + 2*i] << 8) | (uint16_t)rx[10 + 2*i];
    }

    /* Print the buffer */
    printf("\r\nHolding Register Buffer\r\n");
    printf("-----------------------\r\n");

    for(uint16_t i = 0; i < quantity; i++)
    {
        printf("[%03d] = %u\r\n", startAddress + i, HoldingRegBuffer[startAddress + i]);
    }

    printf("\r\n");

    return ERR_OK;
}




static err_t MB_ParseReadInputRegisters(uint8_t *rx)
{
    uint8_t byteCount = rx[8];
    uint16_t quantity = byteCount / 2;
    uint16_t startAddress = MB_CurrentRequest.StartAddress;

    for(uint16_t i = 0; i < quantity; i++)
    {
        InputRegBuffer[startAddress + i] = ((uint16_t)rx[9 + 2*i] << 8) | (uint16_t)rx[10 + 2*i];
    }

    printf("\r\nInput Register Buffer\r\n");
    printf("---------------------\r\n");

    for(uint16_t i = 0; i < quantity; i++)
    {
        printf("[%03d] = %u\r\n", startAddress + i, InputRegBuffer[startAddress + i]);
    }

    printf("\r\n");

    return ERR_OK;
}



static err_t MB_ParseReadCoils(uint8_t *rx)
{
    uint8_t byteCount = rx[8];
    uint16_t startAddress = MB_CurrentRequest.StartAddress;

    for(uint8_t i = 0; i < byteCount; i++)
    {
        for(uint8_t bit = 0; bit < 8; bit++)
        {
            CoilBuffer[startAddress + (i * 8) + bit] = (rx[9 + i] >> bit) & 0x01;
        }
    }

    printf("\r\nCoil Buffer\r\n");
    printf("-----------\r\n");

    for(uint16_t i = 0; i < (byteCount * 8); i++)
    {
        printf("[%03d] = %d\r\n", startAddress + i, CoilBuffer[startAddress + i]);
    }

    printf("\r\n");

    return ERR_OK;
}


static err_t MB_ParseReadDiscreteInputs(uint8_t *rx)
{
    uint8_t byteCount = rx[8];
    uint16_t startAddress = MB_CurrentRequest.StartAddress;

    for(uint8_t i = 0; i < byteCount; i++)
    {
        for(uint8_t bit = 0; bit < 8; bit++)
        {
            InputBuffer[startAddress + (i * 8) + bit] = (rx[9 + i] >> bit) & 0x01;
        }
    }

    printf("\r\nDiscrete Input Buffer\r\n");
    printf("---------------------\r\n");

    for(uint16_t i = 0; i < (byteCount * 8); i++)
    {
        printf("[%03d] = %d\r\n", startAddress + i, InputBuffer[startAddress + i]);
    }

    printf("\r\n");

    return ERR_OK;
}


static err_t MB_ParseException(uint8_t *rx)
{
    uint8_t functionCode  = rx[7] & 0x7F;
    uint8_t exceptionCode = rx[8];

    printf("\r\nModbus Exception Response\r\n");
    printf("-------------------------\r\n");
    printf("Function Code : %d\r\n", functionCode);
    printf("Exception Code: %d - ", exceptionCode);

    switch(exceptionCode)
    {
        case MB_EX_ILLEGAL_FUNCTION:
            printf("Illegal Function");
            break;

        case MB_EX_ILLEGAL_DATA_ADDRESS:
            printf("Illegal Data Address");
            break;

        case MB_EX_ILLEGAL_DATA_VALUE:
            printf("Illegal Data Value");
            break;

        case MB_EX_SERVER_DEVICE_FAILURE:
            printf("Server Device Failure");
            break;

        default:
            printf("Unknown Exception");
            break;
    }

    printf("\r\n\r\n");

    return ERR_OK;
}





