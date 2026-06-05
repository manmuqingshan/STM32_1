/*
 * W25Qxx.h
 *
 *  Created on: May 30, 2026
 *      Author: controllerstech
 */

#ifndef W25QXX_H
#define W25QXX_H

#include "main.h"

#define W25Q_PAGE_SIZE      256
#define W25Q_SECTOR_SIZE    4096

uint32_t W25Q_ReadID(void);

void W25Q_EraseSector(uint32_t sector);

void W25Q_ReadBytes(uint32_t address, uint8_t *buffer, uint32_t length);

void W25Q_WriteBytes(uint32_t address, uint8_t *data, uint32_t length);


#endif /* W25QXX_H_ */
