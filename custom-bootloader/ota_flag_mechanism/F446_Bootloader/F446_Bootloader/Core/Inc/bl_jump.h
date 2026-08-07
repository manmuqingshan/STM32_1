/*
 * bl_jump.h
 *
 *  Created on: Dec 15, 2025
 *      Author: arunr
 */

#ifndef INC_BL_JUMP_H_
#define INC_BL_JUMP_H_

#include <stdint.h>

void JumpToApplication(void);
int bootloader_is_app_valid(void);

#endif /* INC_BL_JUMP_H_ */
