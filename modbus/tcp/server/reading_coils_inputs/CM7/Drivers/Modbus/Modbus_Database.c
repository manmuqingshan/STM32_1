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




uint16_t MB_Coils[MB_COIL_COUNT] = {
		0x12, 0x34, 0x0D
};

uint16_t MB_DisInputs[MB_DIS_INPUT_COUNT] = {0};

static void MB_SetDiscreteInput(uint16_t address, int state)
{
    uint16_t byte = address / 8;
    uint8_t bit = address % 8;

    if(state)
        MB_DisInputs[byte] |= (1 << bit);
    else
        MB_DisInputs[byte] &= ~(1 << bit);
}

void MB_UpdateDiscreteInputs(void)
{
    MB_SetDiscreteInput(0, HAL_GPIO_ReadPin(INPUT1_GPIO_Port, INPUT1_Pin));
    MB_SetDiscreteInput(1, HAL_GPIO_ReadPin(INPUT2_GPIO_Port, INPUT2_Pin));
    MB_SetDiscreteInput(2, HAL_GPIO_ReadPin(INPUT3_GPIO_Port, INPUT3_Pin));
    MB_SetDiscreteInput(3, HAL_GPIO_ReadPin(INPUT4_GPIO_Port, INPUT4_Pin));
    MB_SetDiscreteInput(4, HAL_GPIO_ReadPin(INPUT5_GPIO_Port, INPUT5_Pin));
}
