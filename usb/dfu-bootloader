# STM32 USB DFU Bootloader — Flash Firmware Without ST-Link

Part 11 of the STM32 USB series. This project triggers the STM32's built-in DFU bootloader from firmware using a USB CDC command and an RTC backup register, then flashes new firmware with STM32CubeProgrammer over the same USB cable — no ST-Link or BOOT0 pin required.

---

## 📺 Video Tutorial

[STM32 USB DFU Bootloader — Video Tutorial | USB Series #11](https://youtu.be/OrK7cN96Vl8)

## 📖 Full Article

[STM32 USB DFU Bootloader: Flash Firmware Over USB Without ST-Link](https://controllerstech.com/stm32-usb-dfu-bootloader/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32F446 (WeAct Studio) |
| Interface | USB OTG FS — Device Only mode, CDC class |
| Debug | Serial Wire Debug (SWD) — used only for the first flash |

### Pin Assignment

| Signal | Pin | Function |
|--------|-----|----------|
| Status LED | PB7 | GPIO Output — blinks to confirm new firmware after each flash |
| USB D− | PA11 | USB OTG FS (auto-assigned) |
| USB D+ | PA12 | USB OTG FS (auto-assigned) |

> No BOOT0 wiring is needed for this project. The firmware handles the jump into the DFU bootloader on its own.

---

## What This Project Does

- Configures USB OTG FS in Device mode using the Communication Device Class (CDC)
- Routes `printf` output through the USB CDC interface for logging
- Listens for the `"ENTER DFU"` command inside `CDC_Receive_FS()`
- Writes a marker value to the RTC backup register (`RTC->BKP0R`) and calls `NVIC_SystemReset()` when the command is received
- Checks the backup register at the very start of `main()`, before any peripheral initialization
- Jumps into the STM32's system memory bootloader (`jumpToBootloader()`) when the marker is found, clearing the register first so the next reset boots normally
- Blinks an onboard LED to visually confirm that a new firmware update took effect after flashing

---

## How It Works

Every STM32 microcontroller ships with a DFU bootloader stored in system memory, separate from the application flash. This bootloader configures USB on its own, so no USB setup is required to use it. The only requirement is a way to jump into it.

A direct function call into system memory from a running application is not safe, since peripherals and the vector table are already set up for the application. Instead, the STM32 resets first, and the jump into system memory happens immediately after that reset, before the application initializes anything.

To make this decision persist across the reset, the project uses the RTC backup register. This register survives resets and power loss, similar to flash, but without the overhead of unlocking, writing, and erasing.

### Triggering the Jump

`CDC_Receive_FS()` in `usbd_cdc_if.c` compares incoming USB CDC data against the string `"ENTER DFU"`. On a match, it writes `0x1234` into `RTC->BKP0R` and calls `NVIC_SystemReset()`.

### Checking After Reset

At the very start of `main()`, before `HAL_Init()`, the power controller clock and backup domain access are enabled, and `RTC->BKP0R` is checked. If it holds `0x1234`, the register is cleared immediately and `jumpToBootloader()` is called.

### The Bootloader Jump

`jumpToBootloader()` disables interrupts, resets the SysTick registers, remaps system memory to address `0x00000000`, reads the reset vector from the bootloader's vector table, sets the stack pointer, and branches to the bootloader's entry point.

> The system memory address (`0x1FFF0000` for the STM32F446) is chip-specific. Check the AN2606 application note from ST for the correct address before using this on a different STM32 series.

### Key Files

| File | What it does |
|------|--------------|
| `main.c` | `_write()` for printf over USB CDC, backup register check at boot, `jumpToBootloader()` |
| `usbd_cdc_if.c` | `CDC_Receive_FS()` — matches the `"ENTER DFU"` command and triggers the reset |

---

## CubeMX Configuration Summary

| Peripheral | Setting |
|------------|---------|
| USB OTG FS | Device Only mode |
| USB Device Class | Communication Device Class (CDC) |
| SYS | Serial Wire Debug enabled |
| GPIO | PB7 — Output, default Low (status LED) |
| RTC | Not enabled in CubeMX — backup register accessed directly via `PWR` and `RTC` registers |

### Clock

| Setting | Value |
|---------|-------|
| HSI | 16 MHz internal oscillator |
| System Clock | 80 MHz (via PLL) |
| USB Clock | 48 MHz (HSI48 dedicated oscillator) |

---

## Testing

Flash the project once using SWD, then open a serial monitor on the USB CDC virtual COM port at **115200 baud, 8N1**.

The console prints a log message and the onboard LED blinks every 100 ms, confirming normal operation:
```
Running Test 3..
Running Test 3..
Running Test 3..
```

**Entering DFU mode:** Send the string `ENTER DFU` from the serial terminal. The board resets, disconnects from the serial monitor, and shows up as a USB DFU device in STM32CubeProgrammer.

**Flashing over DFU:** In STM32CubeProgrammer, connect to the detected USB device, browse to the project's Debug folder, select the ELF file, enable "Run after programming," and click Start Programming. The board resets automatically and runs the new firmware, with no SWD connection involved.

**Exiting DFU mode without flashing:** Reset the board. Since the backup register is cleared before the jump, the next reset boots from flash and resumes normal operation.

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
| Part 8 | USB Host MSC — Read & Write USB Flash Drive | [Article](https://controllerstech.com/stm32-usb-host-mass-storage-fatfs/) |
| Part 9 | USB Host HID — Keyboard & Mouse | [Article](https://controllerstech.com/stm32-usb-host-hid-keyboard-mouse/) |
| Part 10 | USB Host Audio Class — WAV Player from SD Card | [Article](https://controllerstech.com/stm32-usb-host-audio-class-sd-card/) |
| **Part 11** | **USB DFU Bootloader — Flash Firmware Without ST-Link (this project)** | [Article](https://controllerstech.com/stm32-usb-dfu-bootloader/) |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
