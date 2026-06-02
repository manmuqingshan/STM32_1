# STM32 USB Mass Storage Class — SD Card as External Drive

Part 4 of the STM32 USB series. This project configures an STM32 as a USB Mass Storage Class (MSC) device using STM32CubeMX and HAL. When connected to a computer over USB, the SD card inserted into the STM32 shows up as an external drive — just like a USB flash drive. No custom drivers are needed on the PC side. Windows and macOS recognise the device automatically.

---

## 📺 Video Tutorial

[STM32 USB Mass Storage Class — SD Card as External Drive | USB Series #4](https://youtu.be/-RoX4oJXDTg)

## 📖 Full Article

[STM32 USB Mass Storage Class: SD Card as External Drive](https://controllerstech.com/stm32-usb-mass-storage-class-sd-card/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32F446RE — WeAct Studio |
| SD Card | Any microSD card, formatted FAT32 |
| Interface | SDIO — 4-bit wide bus mode |
| Logging | USB-to-TTL adapter via UART1 (PA9 TX → TTL RX) |

### SDIO Pin Assignment (WeAct Studio STM32F446RE)

| Signal | Pin |
|--------|-----|
| SDIO_CMD | PD2 |
| SDIO_CK | PC12 |
| SDIO_D0 | PC8 |
| SDIO_D1 | PC9 |
| SDIO_D2 | PC10 |
| SDIO_D3 | PC11 |

> Pin assignments depend on your board. Always verify against your board's schematic.

---

## What This Project Does

- Configures USB OTG FS in Device Only mode using STM32CubeMX
- Enables the USB Mass Storage Class (MSC) middleware with 512-byte media packet buffer
- Connects an SD card via SDIO in 4-bit wide bus mode at 24 MHz
- Implements all six `usbd_storage_if.c` functions for SD card read/write
- Exposes the SD card to the host as a standard block storage device
- Logs GetCapacity, Read, and Write activity over UART1 for debugging

---

## How It Works

The USB Mass Storage Class makes the STM32 appear to the host as a block storage device. The host sends standard SCSI commands — read block, write block, get capacity — and the STM32 translates those into HAL SD card calls.

All the logic lives in one file: `usbd_storage_if.c`. This file bridges the USB stack and the actual storage. Swap out the six functions inside it, and you can use any storage medium — RAM, SD card, or SPI flash.

### The Six Bridge Functions

| Function | Purpose |
|----------|---------|
| `STORAGE_Init_FS` | Initialize the storage medium |
| `STORAGE_GetCapacity_FS` | Report total block count and block size to host |
| `STORAGE_IsReady_FS` | Check SD card state before each transfer |
| `STORAGE_IsWriteProtected_FS` | Tell host if writes are allowed |
| `STORAGE_Read_FS` | Read blocks from SD card into USB buffer |
| `STORAGE_Write_FS` | Write blocks from USB buffer to SD card |

---

## Key Implementation Details

### Why 512 bytes?

SD cards transfer data in fixed 512-byte sectors. The `media packet buffer size` in CubeMX must be set to 512 to match. Anything smaller will break the transfer.

### Why the while loop after ReadBlocks / WriteBlocks?

`HAL_SD_ReadBlocks` and `HAL_SD_WriteBlocks` return before the transfer is complete. Returning from the storage function too early sends incomplete data to the USB host. The while loop waits until the SD card returns to the `HAL_SD_CARD_TRANSFER` state, which confirms the data is fully in the buffer.

```c
if (HAL_SD_ReadBlocks(&hsd, buf, blk_addr, blk_len, HAL_MAX_DELAY) != HAL_OK)
{
    return USBD_FAIL;
}
while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER) {}
```

### Why check card state in IsReady?

Unlike RAM, the SD card can be busy during a transfer. `STORAGE_IsReady_FS` must check the actual card state and return `USBD_FAIL` if the card is not idle — otherwise the host may send a new request while the card is still processing the previous one.

```c
if (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)
{
    return USBD_FAIL;
}
```

---

## Complete usbd_storage_if.c User Code

```c
/* USER CODE BEGIN EXPORTED_VARIABLES */
extern SD_HandleTypeDef hsd;
/* USER CODE END EXPORTED_VARIABLES */


int8_t STORAGE_Init_FS(uint8_t lun)
{
    UNUSED(lun);
    return (USBD_OK);
}


int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    UNUSED(lun);
    HAL_SD_CardInfoTypeDef cardInfo;
    if (HAL_SD_GetCardInfo(&hsd, &cardInfo) != HAL_OK)
    {
        return USBD_FAIL;
    }
    *block_num  = cardInfo.LogBlockNbr - 1;
    *block_size = cardInfo.LogBlockSize;
    printf("Blocks=%lu Size=%u\r\n", *block_num, *block_size);
    return (USBD_OK);
}


int8_t STORAGE_IsReady_FS(uint8_t lun)
{
    UNUSED(lun);
    if (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER)
    {
        return USBD_FAIL;
    }
    return (USBD_OK);
}


int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
    UNUSED(lun);
    return (USBD_OK);
}


int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    UNUSED(lun);
    if (HAL_SD_ReadBlocks(&hsd, buf, blk_addr, blk_len, HAL_MAX_DELAY) != HAL_OK)
    {
        return USBD_FAIL;
    }
    while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER) {}
    if (blk_addr % 1000 == 0)
    {
        printf("READ: ADDR=%lu LEN=%u\r\n", blk_addr, blk_len);
    }
    return (USBD_OK);
}


int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    UNUSED(lun);
    if (HAL_SD_WriteBlocks(&hsd, buf, blk_addr, blk_len, HAL_MAX_DELAY) != HAL_OK)
    {
        return USBD_FAIL;
    }
    while (HAL_SD_GetCardState(&hsd) != HAL_SD_CARD_TRANSFER) {}
    if (blk_addr % 1000 == 0)
    {
        printf("WRITE: ADDR=%lu LEN=%u\r\n", blk_addr, blk_len);
    }
    return (USBD_OK);
}
```

### Serial Logging (_write redirect in main.c)

```c
/* USER CODE BEGIN 0 */
#include "stdio.h"

int _write(int fd, unsigned char *buf, int len)
{
    if (fd == 1 || fd == 2)
    {
        HAL_UART_Transmit(&huart1, buf, len, 999);
    }
    return len;
}
/* USER CODE END 0 */
```

---

## CubeMX Configuration

| Peripheral | Setting |
|------------|---------|
| USB OTG FS | Device Only mode |
| USB Device Class | Mass Storage Class (MSC) |
| Media Packet Buffer Size | 512 bytes |
| SDIO | 4-bit wide bus mode |
| SDIO Clock Divider | 0 (gives 24 MHz SD clock) |
| SDIO Hardware Flow Control | Enabled |
| UART1 | Asynchronous, default baud rate, PA9 TX / PA10 RX |
| SYS | Serial Wire Debug |

### Clock

| Setting | Value |
|---------|-------|
| HSE | External crystal (8 MHz on WeAct Studio board) |
| System Clock | 180 MHz |
| USB Clock | 48 MHz (required — verify in Clock Configuration tab) |

---

## Testing

**Windows / macOS:** Insert a FAT32-formatted SD card into the board. Flash the project and connect the USB cable. The SD card appears as a removable drive in the file explorer. You can copy files to it, delete them, and eject it normally.

**Serial console:** Open a serial terminal at the baud rate configured in CubeMX on the UART1 port. When the USB cable is connected, you will see the `GetCapacity` log. Read and write addresses print every 1000 blocks during transfers.

---

## RAM Storage Demo

The project also includes a commented-out RAM storage demo. This uses 100 KB of the STM32's internal RAM as a fake storage device — useful for verifying the USB Mass Storage Class is working before connecting the SD card.

To enable it, comment out the SD card code and uncomment the RAM buffer and `memcpy` blocks in `usbd_storage_if.c`. See the full article for step-by-step instructions.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. Open the generated project in STM32CubeIDE
4. Build and flash to the board
5. Insert a FAT32-formatted SD card and connect the USB cable

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | CDC — Virtual COM Port | [Article](https://controllerstech.com/stm32-usb-cdc-virtual-com-port/) |
| Part 2 | HID — Gamepad / Joystick | [Article](https://controllerstech.com/stm32-usb-hid-gamepad/) |
| Part 3 | HID — Mouse + Keyboard | [Article](https://controllerstech.com/stm32-usb-hid-mouse-keyboard/) |
| **Part 4** | **MSC — SD Card as External Drive (this project)** | [Article](https://controllerstech.com/stm32-usb-mass-storage-class-sd-card/) |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://ko-fi.com/controllerstech).
