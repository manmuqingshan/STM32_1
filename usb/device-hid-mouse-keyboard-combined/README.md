# STM32 USB HID — Mouse and Keyboard Combined (Single HID Class)

Part 7 of the STM32 USB series. This project configures an STM32 as a single USB HID device that acts as both a mouse and a keyboard at the same time. Instead of using the USB Composite class, it uses one HID interface and splits the data using **Report IDs**. When connected to a computer, the STM32 shows up as one HID device that controls the cursor and types on the keyboard simultaneously, over a single USB cable.

---

## 📺 Video Tutorial

[STM32 USB HID: Mouse and Keyboard Combined — Video Tutorial | USB Series #7](https://youtu.be/QwX5DpZ4UE8)

## 📖 Full Article

[STM32 USB HID: Combine Mouse and Keyboard on One Port](https://controllerstech.com/stm32-usb-hid-mouse-and-keyboard-combined/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32F446RE — WeAct Studio |
| Joystick | Analog joystick module (KY-023 or equivalent) |
| Keypad | 4x4 membrane keypad |
| Buttons | 2x push buttons (left click, right click) |
| Interface | USB OTG FS — Device Only mode |
| ADC | ADC1 Channel 1 (X axis) + Channel 3 (Y axis) with DMA |

### Pin Assignment

| Signal | Pin | Function |
|--------|-----|----------|
| Joystick X (VRx) | PA1 | ADC1 Channel 1 |
| Joystick Y (VRy) | PA3 | ADC1 Channel 3 |
| Joystick switch (Middle click) | PA5 | GPIO Input, Pull-up |
| Left click button | PA7 | GPIO Input, Pull-up |
| Right click button | PC5 | GPIO Input, Pull-up |
| Keypad Row 1 (R1) | PB2 | GPIO Output |
| Keypad Row 2 (R2) | PB0 | GPIO Output |
| Keypad Row 3 (R3) | PC4 | GPIO Output |
| Keypad Row 4 (R4) | PA6 | GPIO Output |
| Keypad Col 1 (C1) | PA4 | GPIO Input, Pull-up |
| Keypad Col 2 (C2) | PA2 | GPIO Input, Pull-up |
| Keypad Col 3 (C3) | PA0 | GPIO Input, Pull-up |
| Keypad Col 4 (C4) | PC2 | GPIO Input, Pull-up |
| USB D− | PA11 | USB OTG FS (auto-assigned) |
| USB D+ | PA12 | USB OTG FS (auto-assigned) |

> Pin assignments depend on your board. Always verify against your board's schematic.

---

## What This Project Does

- Configures USB OTG FS in Device Only mode using STM32CubeMX
- Uses a single USB HID class for both mouse and keyboard data
- Adds a Report ID (`0x85`) to the mouse and keyboard HID descriptors
- Reads joystick X and Y via ADC1 DMA in Circular mode, 8-bit resolution
- Converts ADC values into relative mouse movement (±10 px)
- Reads left, right, and middle mouse buttons via GPIO
- Scans a 4x4 membrane keypad and maps keys to standard HID key codes
- Sends two HID reports per loop — one for the mouse, one for the keyboard
- Demonstrates how to merge two separate HID projects into one

---

## How It Works

A single HID interface can carry more than one report type if each report starts with a **Report ID** byte. The host reads this byte first and routes the rest of the packet to the correct HID function — mouse or keyboard.

This avoids the extra interfaces, endpoints, and Interface Association Descriptor work needed for a Composite USB device (see [Part 6](https://controllerstech.com/stm32-usb-composite-class-cdc-hid/)).

### Report Layout

| Device | Report ID | Total Size | Layout |
|--------|-----------|------------|--------|
| Mouse | `0x01` | 5 bytes | Report ID, Buttons, X, Y, Wheel (unused) |
| Keyboard | `0x02` | 9 bytes | Report ID, Modifier, Reserved, 6x Key codes |

The endpoint size (`HID_EPIN_SIZE`) is set to **9**, since it must fit the larger of the two reports.

### Key Files Modified

| File | What changed |
|------|-------------|
| `usbd_hid.c` | Combined mouse and keyboard descriptors into one array, added `0x85, 0x01` and `0x85, 0x02` Report ID tags |
| `usbd_hid.h` | Updated `HID_MOUSE_REPORT_DESC_SIZE` and `HID_EPIN_SIZE` |
| `main.c` | Added `Mouse_Buffer[5]` and `Keyboard_Buffer[9]`, ADC callback for joystick, `scanKeypad()` function, and main loop sending both reports |

---

## Descriptor Size Calculation (usbd_hid.h)

```c
#define HID_MOUSE_REPORT_DESC_SIZE                 (74U + 43U + 4U)
#define HID_EPIN_SIZE                              0x09U
```

- `74` — mouse descriptor size
- `43` — keyboard descriptor size
- `4` — 2 bytes per Report ID tag, for 2 descriptors

---

## Main Loop Summary (main.c)

```c
while (1)
{
    Mouse_Buffer[0] = 0x01;  // Report ID = 1 for Mouse

    if (HAL_GPIO_ReadPin(L_BUTTON_GPIO_Port, L_BUTTON_Pin) == 0)
        Mouse_Buffer[1] = 0x01;
    if (HAL_GPIO_ReadPin(R_BUTTON_GPIO_Port, R_BUTTON_Pin) == 0)
        Mouse_Buffer[1] = 0x02;
    if (HAL_GPIO_ReadPin(M_BUTTON_GPIO_Port, M_BUTTON_Pin) == 0)
        Mouse_Buffer[1] = 0x04;

    USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)Mouse_Buffer, 5);
    Mouse_Buffer[1] = 0;
    HAL_Delay(5);

    scanKeypad();
    Keyboard_Buffer[0] = 0x02;  // Report ID = 2 for Keyboard
    Keyboard_Buffer[1] = modifier;
    Keyboard_Buffer[3] = keycode;

    USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)Keyboard_Buffer, 9);
    HAL_Delay(5);
}
```

---

## CubeMX Configuration Summary

| Peripheral | Setting |
|------------|---------|
| USB OTG FS | Device Only mode |
| USB Device Class | Human Interface Device (HID) |
| HID_FS_BINTERVAL | 0x0A (10 ms polling) |
| ADC1 | 8-bit, Channel 1 + Channel 3, DMA Circular, Byte width |
| PA5, PA7, PC5 | GPIO Input, Pull-up (mouse buttons) |
| PB2, PB0, PC4, PA6 | GPIO Output (keypad rows R1–R4) |
| PA4, PA2, PA0, PC2 | GPIO Input, Pull-up (keypad columns C1–C4) |
| SYS | Serial Wire Debug |

### Clock

| Setting | Value |
|---------|-------|
| HSE | External crystal (8 MHz) |
| System Clock | 180 MHz |
| USB Clock | 48 MHz (required) |

---

## Testing

Flash the project and connect the STM32 to a PC over USB. The OS should detect it as a standard HID mouse and keyboard — no driver installation needed.

- **Mouse:** Move the joystick to move the cursor. Press the joystick switch for middle click, and the two extra buttons for left/right click.
- **Keyboard:** Press keys on the 4x4 keypad. `0` types Space, `#` types Backspace, and `*` acts as Left Shift.

Both inputs work at the same time, over the same USB connection.

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
| **Part 7** | **HID — Mouse + Keyboard Combined with Report IDs (this project)** | [Article](https://controllerstech.com/stm32-usb-hid-mouse-and-keyboard-combined/) |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://ko-fi.com/controllerstech).
