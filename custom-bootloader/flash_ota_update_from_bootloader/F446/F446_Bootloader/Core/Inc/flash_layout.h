/*
 * flash_layout.h
 *
 *  Created on: Dec 15, 2025
 *      Author: controllerstech
 */

#ifndef INC_FLASH_LAYOUT_H_
#define INC_FLASH_LAYOUT_H_

#define BL_START_ADDR		0x08000000  // 32KB

#define APP_HEADER_ADDR		0x08008000  // 16KB
#define APP_HEADER_SECTOR	FLASH_SECTOR_2

#define APP_START_ADDR		0x0800C000  // 464KB
#define APP_START_SECTOR	FLASH_SECTOR_3
#define APP_END_SECTOR		FLASH_SECTOR_7
#define APP_MAX_SIZE		464*1024


#endif /* INC_FLASH_LAYOUT_H_ */
