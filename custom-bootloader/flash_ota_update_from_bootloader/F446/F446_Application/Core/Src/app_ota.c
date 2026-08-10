/*
 * app_ota.c
 *
 *  Created on: 03-Jan-2026
 *      Author: arunrawat
 */


#include <flash_layout.h>
#include "main.h"
#include "app_ota.h"
#include "flash_operations.h"

#define OTA_FLAG_START		1
#define APP_HEADER_SECTOR	FLASH_SECTOR_2  // Check Ref Manual to see the exact flash num

uint32_t flash_buffer[5];

void enable_ota_request(void)
{
    HAL_FLASH_Unlock();

    flash_read_sector(APP_HEADER_ADDR, flash_buffer);

    /* Update only first word */
    flash_buffer[0] = OTA_FLAG_START;

    flash_erase_sector(APP_HEADER_SECTOR);

    flash_write_sector(APP_HEADER_ADDR, flash_buffer);

    HAL_FLASH_Lock();

    NVIC_SystemReset();
}
