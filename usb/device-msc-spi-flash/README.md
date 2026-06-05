# STM32 USB Mass Storage Class — W25Q NOR Flash as External Drive

Part 5 of the STM32 USB series. This project configures an STM32 as a USB Mass Storage Class (MSC) device using STM32CubeMX and HAL. When connected to a computer over USB, the W25Q SPI NOR Flash module shows up as an external drive — just like a USB flash drive. No custom drivers are needed on the PC side. Windows and macOS recognise the device automatically.

---

## 📺 Video Tutorial

[STM32 USB Mass Storage Class — W25Q NOR Flash as External Drive | USB Series #5](https://youtu.be/zyQI4_cxCmw)

## 📖 Full Article

[STM32 USB Mass Storage Class: W25Q NOR Flash as External Drive](https://controllerstech.com/stm32-usb-mass-storage-class-w25q-nor-flash/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32F446RE — WeAct Studio |
| Flash Module | W25Q16JV — 16 Mbit (2 MB) SPI NOR Flash |
| Interface | SPI1 — Full-Duplex Master |
| Logging | USB-to-TTL adapter via UART1 (PA9 TX → TTL RX) |

### SPI1 Pin Assignment

| Signal | Pin |
|--------|-----|
| SPI1_SCK | PB3 |
| SPI1_MISO | PB4 |
| SPI1_MOSI | PB5 |
| CS (GPIO Output) | PB6 |

> Pin assignments depend on your board. Always verify against your board's schematic.

---

## What This Project Does

- Configures USB OTG FS in Device Only mode using STM32CubeMX
- Enables the USB Mass Storage Class (MSC) middleware with 4096-byte media packet buffer
- Connects a W25Q NOR Flash module via SPI1 at approximately 5 Mbit/s
- Includes a W25Q library with read, write, erase, and ID functions
- Implements all six `usbd_storage_if.c` functions for Flash read/write
- Exposes the Flash module to the host as a standard block storage device
- Logs GetCapacity, Read, and Write activity over UART1 for debugging

---

## How It Works

The USB Mass Storage Class makes the STM32 appear to the host as a block storage device. The host sends standard SCSI commands — read block, write block, get capacity — and the STM32 translates those into W25Q library calls over SPI.

All the logic lives in one file: `usbd_storage_if.c`. This file bridges the USB stack and the actual storage. Swap out the six functions inside it, and you can use any storage medium — RAM, SD card, or SPI flash.

### The Six Bridge Functions

| Function | Purpose |
|----------|---------|
| `STORAGE_Init_FS` | Initialize the storage medium |
| `STORAGE_GetCapacity_FS` | Report total block count and block size to host |
| `STORAGE_IsReady_FS` | Confirm storage is ready for transfer |
| `STORAGE_IsWriteProtected_FS` | Tell host if writes are allowed |
| `STORAGE_Read_FS` | Read blocks from Flash into USB buffer |
| `STORAGE_Write_FS` | Write blocks from USB buffer to Flash |

---

## Key Implementation Details

### Why 4096 bytes instead of 512?

The W25Q Flash memory has a native sector size of 4096 bytes (4 KB). Setting the media packet buffer size in CubeMX to 4096 aligns with this, so the host reads or writes exactly one sector at a time. This also means only one erase operation is needed per write cycle, which simplifies the write function.

### Why erase before every write?

NOR Flash memory cannot overwrite existing data. Bits can only be changed from 1 to 0 during a write. To reset them back to 1, the entire sector must be erased first. So every write operation in `STORAGE_Write_FS` starts with `W25Q_EraseSector` before writing any data.

```c
W25Q_EraseSector(blk_addr + b);
for (uint32_t i = 0; i < 4096; i += 256) {
    W25Q_WriteBytes((blk_addr + b) * 4096 + i, &buf[b * 4096 + i], 256);
}
```

### Why write 256 bytes at a time?

The W25Q Flash memory has a page size of 256 bytes. Writing more than 256 bytes in a single operation causes the data to wrap around and overwrite from the beginning of the same page. So each 4 KB sector is written in sixteen 256-byte page writes.

### Why is block count set to 511?

The USB Mass Storage Class requires the block count to be reported as one less than the actual total. The host interprets this value as the index of the last block, not the total count. For 512 blocks (indices 0 to 511), we report 511.

---

## W25Q Library

The library is included in the project. Copy `W25Qxx.c` into `Core/Src` and `W25Qxx.h` into `Core/Inc`.

Two definitions in the source file may need to be adjusted:

```c
#define W25Q_CHIP_SIZE   16     // Size in Megabits — change for your chip
#define W25Q_SPI         hspi1  // SPI instance — change if using SPI2 or SPI3
```

The four functions used in this project:

| Function | Description |
|----------|-------------|
| `W25Q_ReadID()` | Returns the manufacturer and device ID |
| `W25Q_EraseSector(sector)` | Erases the specified 4 KB sector |
| `W25Q_ReadBytes(addr, buf, len)` | Reads `len` bytes starting from `addr` |
| `W25Q_WriteBytes(addr, buf, len)` | Writes up to 256 bytes starting from `addr` |

---

## Complete usbd_storage_if.c User Code

```c
/* USER CODE BEGIN INCLUDE */
#include "W25Qxx.h"
/* USER CODE END INCLUDE */


int8_t STORAGE_Init_FS(uint8_t lun)
{
    UNUSED(lun);
    return (USBD_OK);
}


int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    UNUSED(lun);
    *block_num  = 511;
    *block_size = 4096;
    printf("Blocks=%lu Size=%u\r\n", *block_num, *block_size);
    return (USBD_OK);
}


int8_t STORAGE_IsReady_FS(uint8_t lun)
{
    UNUSED(lun);
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
    W25Q_ReadBytes(blk_addr * 4096, buf, blk_len * 4096);
    if (blk_addr % 100 == 0)
    {
        printf("READ: ADDR=%lu LEN=%u\r\n", blk_addr, blk_len);
    }
    return (USBD_OK);
}


int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    UNUSED(lun);
    for (uint32_t b = 0; b < blk_len; b++)
    {
        W25Q_EraseSector(blk_addr + b);
        for (uint32_t i = 0; i < 4096; i += 256)
        {
            W25Q_WriteBytes((blk_addr + b) * 4096 + i, &buf[b * 4096 + i], 256);
        }
    }
    if (blk_addr % 100 == 0)
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
| Media Packet Buffer Size | 4096 bytes |
| SPI1 | Full-Duplex Master, 8-bit, MSB First |
| SPI1 Clock Polarity (CPOL) | Low |
| SPI1 Clock Phase (CPHA) | 1 Edge |
| SPI1 Baud Rate | ~5 Mbit/s |
| PB6 (CS) | GPIO Output, default High, speed Very High |
| UART1 | Asynchronous, 115200 baud, PA9 TX / PA10 RX |
| SYS | Serial Wire Debug |

### Clock

| Setting | Value |
|---------|-------|
| HSE | External crystal (8 MHz on WeAct Studio board) |
| System Clock | 180 MHz |
| USB Clock | 48 MHz (required — verify in Clock Configuration tab) |

---

## Testing

**Windows / macOS:** Flash the project and connect the USB cable. The operating system will prompt you to format the drive — this is a one-time operation. Format with FAT32 and Master Boot Record. The W25Q Flash drive then appears as a removable drive in the file explorer. You can copy files to it, delete them, and eject it normally.

**Power retention:** Files written to the W25Q Flash module are retained even after a complete power loss. Reconnect the board after removing all power and the files will still be present — this is the expected behaviour of non-volatile NOR Flash memory.

**Serial console:** Open a serial terminal at 115200 baud on the UART1 port. When the USB cable is connected, you will see the `GetCapacity` log. Read and write addresses print every 100 blocks during transfers.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. Open the generated project in STM32CubeIDE
4. Copy `W25Qxx.c` into `Core/Src` and `W25Qxx.h` into `Core/Inc`
5. Build and flash to the board
6. Connect the USB cable and format the drive when prompted

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | CDC — Virtual COM Port | [Article](https://controllerstech.com/stm32-usb-cdc-virtual-com-port/) |
| Part 2 | HID — Gamepad / Joystick | [Article](https://controllerstech.com/stm32-usb-hid-gamepad/) |
| Part 3 | HID — Mouse + Keyboard | [Article](https://controllerstech.com/stm32-usb-hid-mouse-keyboard/) |
| Part 4 | MSC — SD Card as External Drive | [Article](https://controllerstech.com/stm32-usb-mass-storage-class-sd-card/) |
| **Part 5** | **MSC — W25Q NOR Flash as External Drive (this project)** | [Article](https://controllerstech.com/stm32-usb-mass-storage-class-w25q-nor-flash/) |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://ko-fi.com/controllerstech).
