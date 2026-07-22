/*
 * Modbus_Database.h
 *
 *  Created on: 29-Jun-2026
 *      Author: arunrawat
 */

#ifndef MODBUS_MODBUS_DATABASE_H_
#define MODBUS_MODBUS_DATABASE_H_

#include "stdint.h"

#define MB_HOLDING_REG_COUNT    10
#define MB_INPUT_REG_COUNT      10
#define MB_COIL_COUNT      		20
#define MB_DIS_INPUT_COUNT      5

extern uint16_t MB_HoldingRegs[MB_HOLDING_REG_COUNT];
extern uint16_t MB_InputRegs[MB_INPUT_REG_COUNT];
extern uint16_t MB_Coils[MB_COIL_COUNT];
extern uint16_t MB_DisInputs[MB_DIS_INPUT_COUNT];

#endif /* MODBUS_MODBUS_DATABASE_H_ */
