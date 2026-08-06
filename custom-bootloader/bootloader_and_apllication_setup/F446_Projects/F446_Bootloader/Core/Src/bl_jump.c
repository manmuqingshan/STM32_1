/*
 * bl_jump.c
 *
 *  Created on: Dec 15, 2025
 *      Author: arunr
 */


#include "main.h"
#include "bl_jump.h"
#include "flash_layout.h"

typedef void (*pFunction)(void);

#define APP_MAGIC	0xFEDCBAFE

void JumpToApplication(void)
{
    uint32_t appStack;
    uint32_t appResetHandler;
    pFunction appEntry;

    /* Read application stack pointer */
    appStack = *(volatile uint32_t*)APP_START_ADDR;


    /* Read reset handler address */
    appResetHandler = *(volatile uint32_t*)(APP_START_ADDR + 4);
    appEntry = (pFunction)appResetHandler;

    /* Disable interrupts */
    __disable_irq();

    /* Stop SysTick */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL  = 0;

    /* Set main stack pointer */
    __set_MSP(appStack);

    /* Jump to application reset handler */
    appEntry();
}

