# STM32 USB Composite Class — CDC + HID Gamepad
 
Part 6 of the STM32 USB series. This project configures an STM32 as a USB Composite Class device using STM32CubeMX and HAL. When connected to a computer over USB, the STM32 appears simultaneously as a CDC virtual COM port and an HID gamepad — both over a single USB cable. No custom drivers are needed on the PC side.
 
---
 
## 📺 Video Tutorial
 
[STM32 USB Composite Class: CDC + HID Gamepad Over a Single USB Port | USB Series #6](https://youtu.be/0lwWwdq1zxk)
 
## 📖 Full Article
 
[STM32 USB Composite Class: CDC + HID Gamepad on One USB Port](https://controllerstech.com/stm32-usb-composite-class-cdc-hid/)
 
---
 
## Hardware Used
 
| Component | Details |
|-----------|---------|
| Board | STM32F446RE — WeAct Studio |
| Joystick | Analog joystick module (KY-023 or equivalent) |
| Interface | USB OTG FS — Device Only mode |
| ADC | ADC1 Channel 0 (X axis) + Channel 1 (Y axis) with DMA |
 
### Pin Assignment
 
| Signal | Pin | Function |
|--------|-----|---------|
| VRx (X axis) | PA0 | ADC1 Channel 0 |
| VRy (Y axis) | PA1 | ADC1 Channel 1 |
| SW (Button) | PA2 | GPIO Input with Pull-Up |
| USB D− | PA11 | USB OTG FS (auto-assigned) |
| USB D+ | PA12 | USB OTG FS (auto-assigned) |
 
> Pin assignments depend on your board. Always verify against your board's schematic.
 
---
 
## What This Project Does
 
- Configures USB OTG FS in Device Only mode using STM32CubeMX
- Enables the USB Composite Class middleware
- Registers both CDC and HID using `USBD_RegisterClassComposite`
- Assigns Class IDs using `USBD_CMPSIT_SetClassID` (CDC = 0, HID = 1)
- Reads joystick X and Y via ADC1 DMA in Circular mode
- Sends HID gamepad reports to the computer (axes + button)
- Prints live X, Y, and button values over CDC using `printf`
- Configures the USB TX FIFO for all four active endpoints
 
---
 
## How It Works
 
The STM32 USB Composite Class allows multiple USB classes to share the same USB peripheral. Each class gets its own set of endpoints and a Class ID. The host enumerates them independently — Windows sees a COM port and a game controller at the same time.
 
### Class Registration Order
 
| Class | Endpoint Array | Class ID |
|-------|---------------|---------|
| CDC | 0x81, 0x01, 0x82 | 0 |
| HID | 0x83 | 1 |
 
Class IDs are assigned in registration order by `USBD_CMPSIT_SetClassID`. The returned ID is stored and passed to all class-specific functions.
 
### Key Files Modified
 
| File | What changed |
|------|-------------|
| `usbd_conf.h` | Added composite defines and all endpoint addresses |
| `usbd_device.c` | Replaced `USBD_RegisterClass` with `USBD_RegisterClassComposite` for both classes |
| `usbd_cdc_if.c` | Added Class ID parameter to all CDC functions |
| `usbd_conf.c` | Extended TX FIFO configuration to cover endpoints 0–3 |
| `main.c` | Added HID report send + CDC printf in the main loop |
 
---
 
## Key Implementation Details
 
### Why define all endpoints in usbd_conf.h?
 
CDC uses endpoints `0x81`, `0x01`, and `0x82`. HID originally used `0x81` in the single-class project, which conflicts with CDC. By centralising all endpoint definitions in `usbd_conf.h`, conflicts become immediately visible. HID is moved to `0x83`.
 
### Why does USBD_HID_SendReport block?
 
CubeMX generates a TX FIFO configuration for only two endpoints. In a composite project with four active endpoints, the FIFO for endpoint 3 (HID) is never allocated. The send function stalls waiting for a buffer. The fix is to add `HAL_PCDEx_SetTxFiFo` calls for endpoints 2 and 3 in `usbd_conf.c`.
 
### What is USBD_COMPOSITE_USE_IAD?
 
The Interface Association Descriptor (IAD) tells Windows to group the two CDC interfaces (data + control) together as a single logical device. Without it, Windows may not recognise the CDC interface correctly. Always define this when using CDC in a composite setup on Windows.
 
---
 
## TX FIFO Configuration (usbd_conf.c)
 
```c
HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80);
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0, 0x20);
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1, 0x20);
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 2, 0x20);
HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 3, 0x20);
```
 
Total: 128 + 32 + 32 + 32 + 32 = 256 words. The STM32F446 FIFO limit is 320 words.
 
---
 
## Complete usbd_conf.h Additions
 
```c
/* USER CODE BEGIN INCLUDE */
#define USE_USBD_COMPOSITE
#define USBD_CMPSIT_ACTIVATE_HID    1
#define USBD_CMPSIT_ACTIVATE_CDC    1
#define USBD_COMPOSITE_USE_IAD      1
 
#define CDC_IN_EP       0x81U
#define CDC_OUT_EP      0x01U
#define CDC_CMD_EP      0x82U
#define HID_EPIN_ADDR   0x83U
/* USER CODE END INCLUDE */
```
 
---
 
## CubeMX Configuration Summary
 
| Peripheral | Setting |
|------------|---------|
| USB OTG FS | Device Only mode |
| USB Device Class | Communication Device Class (CDC) |
| Max Number of Interfaces | 2 |
| Max Number of Configurations | 1 |
| CDC RX / TX Buffer | 1024 bytes each |
| ADC1 | 8-bit, 2 channels, DMA Circular, Byte width |
| PA2 | GPIO Input, Pull-Up |
| SYS | Serial Wire Debug |
 
### Clock
 
| Setting | Value |
|---------|-------|
| HSE | External crystal (8 MHz) |
| System Clock | 180 MHz |
| USB Clock | 48 MHz (required) |
 
---
 
## How to Build
 
1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. Open the generated project in STM32CubeIDE
4. Copy the `HID` folder from the [HID Gamepad project](https://github.com/controllerstech/STM32-HAL/tree/master/usb/device-hid-gamepad/f446/Middlewares/ST/STM32_USB_Device_Library/Class/HID) into `Middlewares/ST/STM32_USB_Device_Library/Class/`
5. Copy the `CompositeBuilder` folder from the STM32CubeF4 repository into the same location
6. Build and flash to the board
7. Connect the USB cable — a COM port and a gamepad device should both appear
---
 
## Testing
 
**CDC:** Open a serial terminal at any baud rate on the new COM port. X, Y, and button values print every 100 ms.
 
**HID:** Open [Gamepad Tester](https://hardwaretester.com/gamepad) in a browser. The STM32 will appear as a detected controller. Move the joystick to see the axes move. Press the button to see input registered.
 
Both work simultaneously over the same USB cable.
 
---
 
## Series
 
| Part | Topic | Link |
|------|-------|------|
| Part 1 | CDC — Virtual COM Port | [Article](https://controllerstech.com/stm32-usb-cdc-virtual-com-port/) |
| Part 2 | HID — Gamepad / Joystick | [Article](https://controllerstech.com/stm32-usb-hid-gamepad/) |
| Part 3 | HID — Mouse + Keyboard | [Article](https://controllerstech.com/stm32-usb-hid-mouse-keyboard/) |
| Part 4 | MSC — SD Card as External Drive | [Article](https://controllerstech.com/stm32-usb-mass-storage-class-sd-card/) |
| Part 5 | MSC — W25Q NOR Flash as External Drive | [Article](https://controllerstech.com/stm32-usb-mass-storage-class-w25q-nor-flash/) |
| **Part 6** | **Composite Class — CDC + HID Gamepad (this project)** | [Article](https://controllerstech.com/stm32-usb-composite-class-cdc-hid/) |
 
---
 
## License
 
Open source — free to use and modify. If this project helped you, consider [supporting the work](https://ko-fi.com/controllerstech).
