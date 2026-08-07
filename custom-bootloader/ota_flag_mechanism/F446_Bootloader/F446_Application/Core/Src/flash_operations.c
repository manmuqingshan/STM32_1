/*
 * flash_operations.c
 *
 *  Created on: 03-Jan-2026
 *      Author: arunrawat
 */

#include "main.h"
#include "flash_operations.h"

void flash_read_sector(uint32_t sector_start_addr, uint32_t *buffer)
{
    uint32_t *flash_ptr = (uint32_t *)sector_start_addr;

    for (uint32_t i = 0; i < 5; i++)
    {
    	buffer[i] = flash_ptr[i];
    }
}

void flash_erase_sector(uint32_t sector_number)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t error;

    erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector       = sector_number;
    erase.NbSectors    = 1;

    HAL_FLASHEx_Erase(&erase, &error);
}

void flash_write_sector(uint32_t sector_start_addr, uint32_t *buffer)
{
    uint32_t addr = sector_start_addr;

    for (uint32_t i = 0; i < 5; i++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, buffer[i]);
        addr += 4;
    }
}
