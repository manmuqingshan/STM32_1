# STM32 USB Host HID — Keyboard & Mouse
 
Part 9 of the STM32 USB series. This project configures an STM32 Nucleo L496 as a USB Host using the Human Interface Device (HID) class. It reads keypress data from a wired or wireless keyboard, converts it to ASCII, and prints it on the serial monitor. It also reads X/Y axis movement and button states from a wired or wireless mouse and displays them in real time.
 
---
 
## 📺 Video Tutorial
 
[STM32 USB Host HID: Read Keyboard and Mouse Data — Video Tutorial | USB Series #9](https://youtu.be/4G_KPUVHHBU)
 
## 📖 Full Article
 
[STM32 USB Host HID: Interface a Keyboard and Mouse with STM32](https://controllerstech.com/stm32-usb-host-hid-keyboard-mouse/)
 
---
 
## Hardware Used
 
| Component | Details |
|-----------|---------|
| Board | STM32 Nucleo L496ZG |
| Devices tested | Wired USB keyboard, Wired USB mouse, Wireless keyboard receiver, Wireless mouse receiver |
| Interface | USB OTG FS — Host Only mode |
| UART | LPUART1 — ST-Link virtual COM port |
 
### Pin Assignment
 
| Signal | Pin | Function |
|--------|-----|----------|
| USB D− | PA11 | USB OTG FS (auto-assigned) |
| USB D+ | PA12 | USB OTG FS (auto-assigned) |
| VBUS Sense | PA9 | USB VBUS sensing (auto-assigned) |
| USB Power Switch Enable | PG6 | GPIO Output, default High |
| LPUART1 TX | PG7 | ST-Link virtual COM port TX |
| LPUART1 RX | PG8 | ST-Link virtual COM port RX |
 
> PG6 must be set high to enable the USB power switch IC on the Nucleo L496. Without this, the connected USB device will not receive power and will not be detected.
 
---
 
## What This Project Does
 
- Configures USB OTG FS in Host Only mode using STM32CubeMX
- Uses the USB HID Host class to communicate with keyboards and mice
- Reads keypress data from a USB keyboard, converts HID keycodes to ASCII using `USBH_HID_GetASCIICode()`, and prints each key on the serial console
- Reads X/Y movement and left/right/middle button states from a USB mouse and prints them on the serial console
- Fixes the HID mouse byte offset issue caused by devices that prepend a Report ID byte to their HID report
- Prints full USB interface information (class, subclass, protocol, endpoints) for any connected device
- Supports wireless receivers by selecting a specific interface using `USBH_FindInterface()` with a protocol filter
---
 
## How It Works
 
The USB host stack enumerates the connected HID device automatically. Once the device is detected, the `USBH_UserProcess` callback fires with the `HOST_USER_CLASS_ACTIVE` state. After that, every HID report received from the device triggers `USBH_HID_EventCallback()`. Inside this callback, we check the device type and process keyboard or mouse data accordingly.
 
For keyboards, HID keycodes are converted to ASCII using `USBH_HID_GetASCIICode()`. Empty reports sent on key release are filtered out by checking for a null character.
 
For mice, the raw unsigned X and Y values (0–255) are converted to signed movement values using:
 
```c
if (xVal > 127) xVal -= 255;
if (yVal > 127) yVal -= 255;
```
 
This gives positive and negative values for movement in either direction. The mouse reports relative movement, not absolute position.
 
### Byte Offset Fix for Mouse
 
Some mice prepend a Report ID byte to every HID report. This shifts all data one byte to the right compared to what the library expects. The fix is to update the `data` pointer offsets in the `prop_b1`, `prop_b2`, `prop_b3`, `prop_x`, and `prop_y` structures inside `usbh_hid_mouse.c`. Always print the raw buffer first to confirm the actual byte layout of your specific mouse.
 
### Wireless Device Interface Selection
 
Wireless receivers expose multiple interfaces. The HAL USB HID host selects the first one it finds. To force selection of the mouse interface, the last parameter of `USBH_FindInterface()` in `usbh_hid.c` is changed from `0xFF` (any interface) to `0x02` (mouse protocol). For keyboards, leaving it at `0xFF` works fine when the keyboard interface appears first in the descriptor.
 
> The STM32 HAL USB Host HID library supports only one interface at a time. Using both keyboard and mouse from the same wireless receiver simultaneously is not supported without significant library modifications.
 
### Key Files Modified
 
| File | What changed |
|------|--------------|
| `main.c` | Added `_write()` for printf routing, `USBH_HID_EventCallback()` for keyboard and mouse data |
| `usb_host.c` | Added `USB_PrintInterfaces()` to print interface info, state logging in `USBH_UserProcess` |
| `usbh_hid_mouse.c` | Updated byte offsets in `prop_b1`, `prop_b2`, `prop_b3`, `prop_x`, `prop_y` to match the actual HID report layout |
| `usbh_hid.c` | Changed the last parameter of `USBH_FindInterface()` to select mouse or keyboard interface for wireless devices |
 
---
 
## CubeMX Configuration Summary
 
| Peripheral | Setting |
|------------|---------|
| USB OTG FS | Host Only mode |
| USB Host Class | Human Interface Device Host Class (HID) |
| VBUS Sensing | Enabled — PA9 |
| USBH_USE_OS | Disabled (no FreeRTOS) |
| USBH_DEBUG_LEVEL | 0 (set to 3 for verbose USB logs) |
| LPUART1 | 115200 baud, 8N1, TX → PG7, RX → PG8 |
| PG6 | GPIO Output, default High (USB power switch enable) |
 
### Clock
 
| Setting | Value |
|---------|-------|
| HSI | 16 MHz internal oscillator |
| System Clock | 80 MHz (via PLL) |
| USB Clock | 48 MHz (HSI48 dedicated oscillator) |
 
---
 
## Testing
 
Flash the project and open a serial monitor at **115200 baud, 8N1** on the ST-Link virtual COM port.
 
**Keyboard:** Plug in a USB keyboard. The console will print:
```
Device Connected
Device Ready
```
Press any key and the character will appear. Shift and numpad keys work correctly.
 
**Mouse:** Plug in a USB mouse. The console will print:
```
Device Connected
Device Ready
```
Move the mouse and click buttons to see output like:
```
x: 5, y: -3, b0: 0, b1: 0, b2: 0
x: 0, y: 0, b0: 1, b1: 0, b2: 0
```
 
**Wireless devices:** Connect the receiver. Use `USB_PrintInterfaces()` output to identify the interface layout. Change the `USBH_FindInterface()` parameter and byte offsets in `usbh_hid_mouse.c` as needed for your specific receiver.
 
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
| **Part 9** | **USB Host HID — Keyboard & Mouse (this project)** | [Article](https://controllerstech.com/stm32-usb-host-hid-keyboard-mouse/) |
 
---
 
## License
 
Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
