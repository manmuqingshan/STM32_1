/*
 * Modbus_Database.c
 *
 *  Created on: 29-Jun-2026
 *      Author: arunrawat
 */


#include "Modbus_Database.h"
#include "stdint.h"
#include "main.h"

uint16_t MB_HoldingRegs[MB_HOLDING_REG_COUNT] = {
		1000, 12345, 62342, 8954, 0, 65535, 45387, 1, 34, 789,
};

uint16_t MB_InputRegs[MB_INPUT_REG_COUNT] = {0};


extern uint16_t readADC (void);

void MB_UpdateInputRegisters(void)
{
	uint16_t adcVal = readADC();
	MB_InputRegs[0] = HAL_GetTick() / 1000;
    MB_InputRegs[1] = adcVal;
    MB_InputRegs[2] = 0xFFFF-adcVal;
}

