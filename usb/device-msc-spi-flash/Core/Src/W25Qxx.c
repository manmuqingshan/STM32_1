/*
 * W25Qxx.c
 *
 *  Created on: Jul 15, 2023
 *      Author: controllerstech
 */


#include "main.h"
#include "W25Qxx.h"
#include "string.h"

extern SPI_HandleTypeDef hspi1;
#define W25Q_SPI hspi1

#define sizeinMbits 16  // 32x16x16 pages and 32x16x16x256 Bytes

void csLOW (void)
{
	HAL_GPIO_WritePin (CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET);
}

void csHIGH (void)
{
	HAL_GPIO_WritePin (CS_GPIO_Port, CS_Pin, GPIO_PIN_SET);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void W25Q_WriteEnable(void);
static void W25Q_WaitBusy(void);


static void W25Q_WaitBusy(void)
{
    uint8_t cmd = 0x05;
    uint8_t sr;

    do
    {
        csLOW();
        HAL_SPI_Transmit(&W25Q_SPI, &cmd, 1, 100);
        HAL_SPI_Receive(&W25Q_SPI, &sr, 1, 100);
        csHIGH();
    } while (sr & 0x01);
}

static void W25Q_WriteEnable(void)
{
    uint8_t cmd = 0x06;

    csLOW();
    HAL_SPI_Transmit(&W25Q_SPI, &cmd, 1, 100);
    csHIGH();
}

uint32_t W25Q_ReadID(void)
{
    uint8_t cmd = 0x9F;
    uint8_t id[3];

    csLOW();

    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, id, 3, HAL_MAX_DELAY);

    csHIGH();

    return ((uint32_t)id[0] << 16) | ((uint32_t)id[1] << 8)  | ((uint32_t)id[2]);
}


void W25Q_EraseSector(uint32_t sector)
{
	uint8_t cmd[5];
	uint32_t address = sector * W25Q_SECTOR_SIZE;

	if (sizeinMbits<256)   // Chip Size<256Mb
	{
		cmd[0] = 0x20;  // enable Read
		cmd[1] = (address>>16)&0xFF;  // MSB of the memory Address
		cmd[2] = (address>>8)&0xFF;
		cmd[3] = (address)&0xFF; // LSB of the memory Address
	}
	else  // we use 32bit memory address for chips >= 256Mb
	{
		cmd[0] = 0x21;  // ERASE Sector with 32bit address
		cmd[1] = (address>>24)&0xFF;  // MSB of the memory Address
		cmd[2] = (address>>16)&0xFF;
		cmd[3] = (address>>8)&0xFF;
		cmd[4] = (address)&0xFF; // LSB of the memory Address
	}

	W25Q_WriteEnable();
	csLOW();  // pull the CS Low
	if (sizeinMbits<256)
	{
		HAL_SPI_Transmit(&W25Q_SPI, cmd, 4, 100);  // send instruction along with the 24 bit memory address
	}
	else
	{
		HAL_SPI_Transmit(&W25Q_SPI, cmd, 5, 100);  // send instruction along with the 32 bit memory address
	}

	csHIGH();

	W25Q_WaitBusy();

}

void W25Q_ReadBytes(uint32_t address, uint8_t *buffer, uint32_t length)
{
	uint8_t cmd[6];

	if (sizeinMbits<256)   // Chip Size<256Mb
	{
		cmd[0] = 0x0B;  // enable Fast Read
		cmd[1] = (address>>16)&0xFF;  // MSB of the memory Address
		cmd[2] = (address>>8)&0xFF;
		cmd[3] = (address)&0xFF; // LSB of the memory Address
		cmd[4] = 0;  // Dummy clock
	}
	else  // we use 32bit memory address for chips >= 256Mb
	{
		cmd[0] = 0x0C;  // Fast Read with 4-Byte Address
		cmd[1] = (address>>24)&0xFF;  // MSB of the memory Address
		cmd[2] = (address>>16)&0xFF;
		cmd[3] = (address>>8)&0xFF;
		cmd[4] = (address)&0xFF; // LSB of the memory Address
		cmd[5] = 0;  // Dummy clock
	}

	csLOW();
	if (sizeinMbits<256)
	{
		HAL_SPI_Transmit(&W25Q_SPI, cmd, 5, 100);  // send read instruction along with the 24 bit memory address
	}
	else
	{
		HAL_SPI_Transmit(&W25Q_SPI, cmd, 6, 100);  // send read instruction along with the 32 bit memory address
	}

	 HAL_SPI_Receive(&W25Q_SPI, buffer, length, HAL_MAX_DELAY);

//	for(uint32_t i = 0; i < length; i++)
//	    {
//	        uint8_t dummy = 0xFF;
//
//	        HAL_SPI_TransmitReceive(&W25Q_SPI, &dummy, &buffer[i], 1, HAL_MAX_DELAY);
//
//	    }

	csHIGH();
}



void W25Q_WriteBytes(uint32_t address, uint8_t *data, uint32_t length)
{
    uint8_t header[5];

	if (sizeinMbits<256)   // Chip Size<256Mb
	{
		header[0] = 0x02;  // page program
		header[1] = (address>>16)&0xFF;  // MSB of the memory Address
		header[2] = (address>>8)&0xFF;
		header[3] = (address)&0xFF; // LSB of the memory Address
	}

	else // we use 32bit memory address for chips >= 256Mb
	{
		header[0] = 0x12;  // page program with 4-Byte Address
		header[1] = (address>>24)&0xFF;  // MSB of the memory Address
		header[2] = (address>>16)&0xFF;
		header[3] = (address>>8)&0xFF;
		header[4] = (address)&0xFF; // LSB of the memory Address
	}

    W25Q_WriteEnable();

    csLOW();
    if (sizeinMbits<256) HAL_SPI_Transmit(&W25Q_SPI, header, 4, 100);
    else HAL_SPI_Transmit(&W25Q_SPI, header, 5, 100);

    HAL_SPI_Transmit(&W25Q_SPI, data, length, HAL_MAX_DELAY);
    csHIGH();

    W25Q_WaitBusy();
}


