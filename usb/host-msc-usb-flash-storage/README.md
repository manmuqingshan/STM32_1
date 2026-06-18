# STM32 USB Host MSC — Read & Write a USB Flash Drive with FatFs and FreeRTOS

Part 8 of the STM32 USB series. This project configures an STM32 Nucleo L496 as a USB host using the Mass Storage Class. It reads a configuration file from a FAT32 USB flash drive to set the UART baud rate and LED blink delay at runtime, and writes a test file back to the drive — all running under FreeRTOS.

---

## 📺 Video Tutorial

[STM32 USB Host MSC: Read & Write a USB Flash Drive — Video Tutorial | USB Series #8](https://youtu.be/HJpeWK3R4iM)

## 📖 Full Article

[STM32 USB Host MSC: Read & Write USB Flash Drive with FatFs and FreeRTOS](https://controllerstech.com/stm32-usb-host-mass-storage-fatfs/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32 Nucleo L496ZG |
| Storage | FAT32-formatted USB flash drive |
| Interface | USB OTG FS — Host Only mode |
| UART | LPUART1 — ST-Link virtual COM port |
| LED | PB7 — blue LED on the Nucleo board |

### Pin Assignment

| Signal | Pin | Function |
|--------|-----|----------|
| USB D− | PA11 | USB OTG FS (auto-assigned) |
| USB D+ | PA12 | USB OTG FS (auto-assigned) |
| VBUS Sense | PA9 | USB VBUS sensing (auto-assigned) |
| USB Power Switch Enable | PG6 | GPIO Output, default High |
| LPUART1 TX | PG7 | ST-Link virtual COM port TX |
| LPUART1 RX | PG8 | ST-Link virtual COM port RX |
| LED | PB7 | GPIO Output |

> PG6 must be set high to enable the USB power switch IC on the Nucleo L496. Without this, the connected USB device will not receive power and will not be detected.

---

## What This Project Does

- Configures USB OTG FS in Host Only mode using STM32CubeMX
- Uses the USB Mass Storage Class (MSC) to communicate with a FAT32 flash drive
- Mounts and unmounts the flash drive automatically on plug and unplug using FatFs
- Reads `config.txt` from the root of the drive and parses two key-value pairs:
  - `BAUD RATE` — updates the LPUART1 baud rate at runtime
  - `LED DELAY` — updates the LED blink interval at runtime
- Writes a test file (`test.TXT`) to the root of the drive
- Runs three FreeRTOS tasks — USB host init, UART logging, and LED blinking

---

## config.txt Format

Create a plain text file named `config.txt` in the root of your FAT32 flash drive with the following format:
BAUD RATE: "14400"

LED DELAY: "250"

Values must be enclosed in double quotes. The flash drive must be formatted as FAT32 — NTFS and exFAT are not supported by FatFs on STM32.

---

## How It Works

The USB host stack handles enumeration automatically. Once the flash drive is connected, the `USBH_UserProcess` callback fires with the `HOST_USER_CLASS_ACTIVE` state. At that point, FatFs mounts the drive and the application opens `config.txt`, reads it into a buffer, and parses it line by line using `strtok` and `sscanf`. The extracted values are written to shared variables that the UART and LED FreeRTOS tasks read on every iteration.

When the drive is disconnected, the `HOST_USER_DISCONNECTION` state fires and FatFs unmounts cleanly. The UART and LED continue running with whatever values were last applied.

### FreeRTOS Task Layout

| Task | Stack | Role |
|------|-------|------|
| defaultTask | 128 words | Calls `MX_USB_HOST_Init()` at startup |
| uartTask | 512 words | Prints log every 1 s, applies new baud rate when `baudChanged` flag is set |
| ledTask | 128 words | Toggles PB7 at the interval stored in `LED_Delay` |

When FreeRTOS is active, the USB host middleware manages its own internal processing task. `MX_USB_HOST_Process()` does not need to be called manually.

### Key Files Modified

| File | What changed |
|------|--------------|
| `usb_host.c` | Added FatFs mount/unmount, `config.txt` read and parse, `test.TXT` write inside `USBH_UserProcess` |
| `main.c` | Added `_write` for printf routing, shared variables `Baud_Rate`, `LED_Delay`, `baudChanged`, and FreeRTOS task implementations |

---

## CubeMX Configuration Summary

| Peripheral | Setting |
|------------|---------|
| USB OTG FS | Host Only mode |
| USB Host Class | Mass Storage Class (MSC) |
| VBUS Sensing | Enabled — PA9 |
| USB Host Task Stack | 1024 words |
| FatFs | USB Disk, read-only disabled |
| FreeRTOS | CMSIS v2, heap ≥ 20 KB, Newlib Reentrant enabled |
| LPUART1 | 115200 baud, 8N1, TX → PG7, RX → PG8 |
| PG6 | GPIO Output, default High (USB power switch enable) |
| PB7 | GPIO Output (LED) |
| HAL Time Base | TIM6 (SysTick reserved for FreeRTOS) |

### Clock

| Setting | Value |
|---------|-------|
| HSI | 16 MHz internal oscillator |
| System Clock | 80 MHz (via PLL) |
| USB Clock | 48 MHz (HSI48 dedicated oscillator) |

---

## Testing

Flash the project and open a serial monitor at **115200 baud, 8N1** on the ST-Link virtual COM port. With no flash drive connected, the UART prints "Hello World from USB" every second and the LED blinks every 1000 ms.

Plug in the flash drive. The console will show:
Application Start

Application Ready

Mount Successful

File Content

BAUD RATE: "14400"

LED DELAY: "250"
Baud Rate changed to 14400

LED Blink Delay is now 250

Switch the Baud Rate on Console to 14400

20 bytes written to file test.TXT

Switch your serial monitor to **14400 baud** to continue seeing the log output. The LED will now blink at 250 ms. Reconnect the drive to a PC and verify that `test.TXT` has been created in the root with the expected content.

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | CDC — Virtual COM Port | [Article](https://controllerstech.com/stm32-usb-cdc-virtual-com-port/) |
| Part 2 | HID — Gamepad / Joystick | [Article](https://controllerstech.com/stm32-usb-hid-gamepad/) |
| Part 3 | HID — Mouse + Keyboard (separate projects) | [Article](https://controllerstech.com/stm32-usb-hid-mouse-keyboard/) |
| Part 4 | MSC — SD Card as External Drive | [Article](https://controllerstech.com/stm32-usb-mass-storage-class-sd-card/) |
| Part 5 | MSC — W25Q NOR Flash as External Drive | [Article](https://controllerstech.com/stm32-usb-mass-storage-class-w25q-nor-flash/) |
| Part 6 | Composite Class — CDC + HID Gamepad | [Article](https://controllerstech.com/stm32-usb-composite-class-cdc-hid/) |
| Part 7 | HID — Mouse + Keyboard Combined with Report IDs | [Article](https://controllerstech.com/stm32-usb-hid-mouse-and-keyboard-combined/) |
| **Part 8** | **USB Host MSC — Read & Write USB Flash Drive (this project)** | [Article](https://controllerstech.com/stm32-usb-host-mass-storage-fatfs/) |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
