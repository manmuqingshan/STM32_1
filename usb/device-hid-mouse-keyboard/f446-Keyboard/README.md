# STM32 USB HID — Keyboard

Part 3 (Keyboard) of the STM32 USB series. This project configures an STM32 as a USB HID keyboard using STM32CubeMX and HAL. The default mouse descriptor is replaced with a custom keyboard descriptor. A 4×4 membrane keypad is scanned using a row-pull-low method, and each key press is sent to the host as a standard HID key code. No custom drivers are needed on the PC side — Windows and macOS recognise the device automatically as a standard keyboard.

---

## 📺 Video Tutorial

[STM32 USB HID Mouse and Keyboard — HID Device Implementation | USB Series #3](https://youtu.be/5SkVv5UY8cw)

## 📖 Full Article

[STM32 USB HID: Emulate a Mouse and Keyboard with STM32](https://controllerstech.com/stm32-usb-hid-mouse-keyboard/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32F446 — weAct Studio |
| Keypad | 4×4 membrane keypad — 4 row pins, 4 column pins |
| Row 1 | PB1 — GPIO Output |
| Row 2 | PC5 — GPIO Output |
| Row 3 | PA7 — GPIO Output |
| Row 4 | PA5 — GPIO Output |
| Col 1 | PA3 — GPIO Input with internal pull-up |
| Col 2 | PA1 — GPIO Input with internal pull-up |
| Col 3 | PC3 — GPIO Input with internal pull-up |
| Col 4 | PC1 — GPIO Input with internal pull-up |

---

## What This Project Does

- Configures USB OTG FS in Device Only mode using STM32CubeMX
- Replaces the default mouse HID report descriptor with a standard 8-byte keyboard descriptor
- Scans a 4×4 membrane keypad using the row-pull-low method
- Maps each key to an HID key code from the HID Usage Tables document
- Supports modifier keys — the `*` key is mapped to Left Shift
- Sends an 8-byte HID report (modifier, reserved, 6 key codes) to the host every 10 ms

---

## HID Report Structure

The keyboard sends 8 bytes per report:

| Byte | Content | Notes |
|------|---------|-------|
| 0 | Modifier keys | Bitmask — Bit 1 = Left Shift, Bit 0 = Left Ctrl, etc. |
| 1 | Reserved | Always 0x00 |
| 2 | Key code 1 | First active key — HID usage ID from Usage Tables |
| 3–7 | Key codes 2–6 | Additional simultaneous keys — unused in this project (0x00) |

---

## Descriptor Changes

CubeMX generates a mouse descriptor by default. This must be replaced with the keyboard descriptor below in `usbd_hid.c`. Also update `usbd_hid.h` to match.

### usbd_hid.c — Replace descriptor array contents

```c
__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE] __ALIGN_END =
{
  0x05, 0x01,  /* Usage Page (Generic Desktop Ctrls) */
  0x09, 0x06,  /* Usage (Keyboard)                   */
  0xA1, 0x01,  /* Collection (Application)           */
  0x05, 0x07,  /*   Usage Page (Keyboard)            */

  /* Modifier Byte (1 Byte) */
  0x19, 0xE0,  /*   Usage Minimum (Left Ctrl  = 0xE0) */
  0x29, 0xE7,  /*   Usage Maximum (Right GUI  = 0xE7) */
  0x15, 0x00,  /*   Logical Minimum (0)               */
  0x25, 0x01,  /*   Logical Maximum (1)               */
  0x95, 0x08,  /*   Report Count (8)                  */
  0x75, 0x01,  /*   Report Size (1 bit)               */
  0x81, 0x02,  /*   Input (Data, Var, Abs)            */

  /* Reserved Byte (1 Byte) */
  0x95, 0x01,  /*   Report Count (1)                  */
  0x75, 0x08,  /*   Report Size (8 bits)              */
  0x81, 0x01,  /*   Input (Const, Array, Abs)         */

  /* Key Codes (6 Bytes) */
  0x19, 0x00,  /*   Usage Minimum (0x00)              */
  0x29, 0x65,  /*   Usage Maximum (0x65)              */
  0x15, 0x00,  /*   Logical Minimum (0)               */
  0x25, 0x65,  /*   Logical Maximum (0x65)            */
  0x75, 0x08,  /*   Report Size (8 bits)              */
  0x95, 0x06,  /*   Report Count (6)                  */
  0x81, 0x00,  /*   Input (Data, Array, Abs)          */
  0xC0,        /* End Collection                      */
};
```

### usbd_hid.h — Update defines

```c
#define HID_MOUSE_REPORT_DESC_SIZE    43U
#define HID_EPIN_SIZE                 0x08U
```

> **Reminder:** CubeMX overwrites `usbd_hid.c` and `usbd_hid.h` every time you regenerate the project. Save the descriptor and the two defines separately and re-apply them after each regeneration.

---

## Key Mapping

| Keypad Key | Row | Col | HID Key Code | Notes |
|------------|-----|-----|--------------|-------|
| 1 | R1 | C1 | 0x1E | Number 1 |
| 2 | R1 | C2 | 0x1F | Number 2 |
| 3 | R1 | C3 | 0x20 | Number 3 |
| A | R1 | C4 | 0x04 | Letter A |
| 4 | R2 | C1 | 0x21 | Number 4 |
| 5 | R2 | C2 | 0x22 | Number 5 |
| 6 | R2 | C3 | 0x23 | Number 6 |
| B | R2 | C4 | 0x05 | Letter B |
| 7 | R3 | C1 | 0x24 | Number 7 |
| 8 | R3 | C2 | 0x25 | Number 8 |
| 9 | R3 | C3 | 0x26 | Number 9 |
| C | R3 | C4 | 0x06 | Letter C |
| * | R4 | C1 | — | Left Shift (modifier, bit 1) |
| 0 | R4 | C2 | 0x2C | Space |
| # | R4 | C3 | 0x2A | Backspace |
| D | R4 | C4 | 0x07 | Letter D |

Key codes are taken from the [HID Usage Tables](https://usb.org/sites/default/files/hut1_5.pdf) document, Keyboard/Keypad section.

---

## How Keypad Scanning Works

All four row pins are set HIGH by default. To scan, one row is pulled LOW at a time. If a key in that row is pressed, the column pin connected to that key is also pulled LOW (through the key contact). Reading all four column pins tells us which key was pressed.

```
Row active (LOW) + Column reads LOW → key at that intersection is pressed
Row active (LOW) + Column reads HIGH → no key pressed in that column
```

A 1 ms delay after setting each row allows the GPIO pin to settle before reading the columns.

---

## Functions / API Used

| Function | File | Description |
|----------|------|-------------|
| `scanKeypad()` | `main.c` | Scans all 16 keys and updates `modifier` and `keycode` variables |
| `USBD_HID_SendReport()` | `usbd_hid.c` | Send an 8-byte HID report from STM32 to host |
| `HAL_GPIO_WritePin()` | `main.c` | Sets/resets row pins to activate one row at a time |
| `HAL_GPIO_ReadPin()` | `main.c` | Reads column pins to detect which key is pressed |

---

## Project Structure

```
keyboard/
├── Core/
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       └── main.c              ← scanKeypad(), HID buffer fill, send loop
├── USB_DEVICE/
│   ├── App/
│   │   └── usbd_desc.c
│   └── Target/
│       └── usbd_conf.c
├── Middlewares/
│   └── ST/STM32_USB_Device_Library/Class/HID/
│       ├── Src/
│       │   └── usbd_hid.c      ← Modified: keyboard report descriptor (43 bytes)
│       └── Inc/
│           └── usbd_hid.h      ← Modified: DESC_SIZE = 43, EPIN_SIZE = 0x08
├── Drivers/
└── .ioc                        ← STM32CubeMX project file
```

---

## CubeMX Configuration

| Peripheral | Pin | Mode / Setting |
|------------|-----|----------------|
| USB OTG FS | PA11, PA12 | Device Only |
| GPIO Output | PB1 | R1 — Initial level: High |
| GPIO Output | PC5 | R2 — Initial level: High |
| GPIO Output | PA7 | R3 — Initial level: High |
| GPIO Output | PA5 | R4 — Initial level: High |
| GPIO Input | PA3 | C1 — Pull-up enabled |
| GPIO Input | PA1 | C2 — Pull-up enabled |
| GPIO Input | PC3 | C3 — Pull-up enabled |
| GPIO Input | PC1 | C4 — Pull-up enabled |

### Clock

| Setting | Value |
|---------|-------|
| System Clock | 180 MHz |
| USB Clock | 48 MHz (required — verify in Clock Configuration tab) |

### Important GPIO Settings

- **Row pins (R1–R4):** Set the initial GPIO output level to **High** in the GPIO configuration tab. All rows must be high by default; scanning pulls them low one at a time.
- **Column pins (C1–C4):** Enable the internal **Pull-up** resistor for each column pin. This ensures columns read high when no key is pressed.

### Product Name

Set the product name to **STM32 Keyboard** in the Device Descriptor tab before generating code.

---

## Complete main.c User Code

```c
/* USER CODE BEGIN 0 */
extern USBD_HandleTypeDef hUsbDeviceFS;
uint8_t HID_Buffer[8] = {0};
uint8_t modifier = 0;
uint8_t keycode  = 0;

void scanKeypad(void)
{
    modifier = keycode = 0;

    /* Row 1 */
    HAL_GPIO_WritePin(R1_GPIO_Port, R1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(R2_GPIO_Port, R2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(R3_GPIO_Port, R3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(R4_GPIO_Port, R4_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    if (HAL_GPIO_ReadPin(C1_GPIO_Port, C1_Pin) == GPIO_PIN_RESET) keycode = 0x1E; // 1
    if (HAL_GPIO_ReadPin(C2_GPIO_Port, C2_Pin) == GPIO_PIN_RESET) keycode = 0x1F; // 2
    if (HAL_GPIO_ReadPin(C3_GPIO_Port, C3_Pin) == GPIO_PIN_RESET) keycode = 0x20; // 3
    if (HAL_GPIO_ReadPin(C4_GPIO_Port, C4_Pin) == GPIO_PIN_RESET) keycode = 0x04; // A

    /* Row 2 */
    HAL_GPIO_WritePin(R1_GPIO_Port, R1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(R2_GPIO_Port, R2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(R3_GPIO_Port, R3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(R4_GPIO_Port, R4_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    if (HAL_GPIO_ReadPin(C1_GPIO_Port, C1_Pin) == GPIO_PIN_RESET) keycode = 0x21; // 4
    if (HAL_GPIO_ReadPin(C2_GPIO_Port, C2_Pin) == GPIO_PIN_RESET) keycode = 0x22; // 5
    if (HAL_GPIO_ReadPin(C3_GPIO_Port, C3_Pin) == GPIO_PIN_RESET) keycode = 0x23; // 6
    if (HAL_GPIO_ReadPin(C4_GPIO_Port, C4_Pin) == GPIO_PIN_RESET) keycode = 0x05; // B

    /* Row 3 */
    HAL_GPIO_WritePin(R1_GPIO_Port, R1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(R2_GPIO_Port, R2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(R3_GPIO_Port, R3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(R4_GPIO_Port, R4_Pin, GPIO_PIN_SET);
    HAL_Delay(1);
    if (HAL_GPIO_ReadPin(C1_GPIO_Port, C1_Pin) == GPIO_PIN_RESET) keycode = 0x24; // 7
    if (HAL_GPIO_ReadPin(C2_GPIO_Port, C2_Pin) == GPIO_PIN_RESET) keycode = 0x25; // 8
    if (HAL_GPIO_ReadPin(C3_GPIO_Port, C3_Pin) == GPIO_PIN_RESET) keycode = 0x26; // 9
    if (HAL_GPIO_ReadPin(C4_GPIO_Port, C4_Pin) == GPIO_PIN_RESET) keycode = 0x06; // C

    /* Row 4 */
    HAL_GPIO_WritePin(R1_GPIO_Port, R1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(R2_GPIO_Port, R2_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(R3_GPIO_Port, R3_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(R4_GPIO_Port, R4_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    if (HAL_GPIO_ReadPin(C1_GPIO_Port, C1_Pin) == GPIO_PIN_RESET) modifier |= (1 << 1); // Left Shift
    if (HAL_GPIO_ReadPin(C2_GPIO_Port, C2_Pin) == GPIO_PIN_RESET) keycode = 0x2C;       // Space
    if (HAL_GPIO_ReadPin(C3_GPIO_Port, C3_Pin) == GPIO_PIN_RESET) keycode = 0x2A;       // Backspace
    if (HAL_GPIO_ReadPin(C4_GPIO_Port, C4_Pin) == GPIO_PIN_RESET) keycode = 0x07;       // D
}
/* USER CODE END 0 */

/* USER CODE BEGIN 3 */
scanKeypad();
HID_Buffer[0] = modifier;  // Byte 0: modifier keys
HID_Buffer[2] = keycode;   // Byte 2: key code (Byte 1 is reserved — stays 0)
USBD_HID_SendReport(&hUsbDeviceFS, (uint8_t *)HID_Buffer, 8);
HAL_Delay(10);
/* USER CODE END 3 */
```

---

## Testing

**Windows:** Open Notepad or any text editor. Press keypad keys — characters should appear. Hold `*` (Left Shift) and press A, B, C, or D — uppercase letters should appear. Press `#` — the last character should be deleted. Press `0` — a space should appear.

**macOS:** Same behaviour. The device appears under Settings → General → System Report → USB as **STM32 Keyboard**.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. **Re-apply descriptor changes** — paste the keyboard descriptor back into `usbd_hid.c` and update both defines in `usbd_hid.h` (CubeMX overwrites these on regeneration)
4. Open the generated project in STM32CubeIDE
5. Build and flash to the board
6. Connect the USB cable to the board's USB connector

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | CDC — Virtual COM Port | [Article](https://controllerstech.com/stm32-usb-cdc-virtual-com-port/) |
| Part 2 | HID — Gamepad / Joystick | [Article](https://controllerstech.com/stm32-usb-hid-gamepad/) |
| **Part 3** | **HID — Mouse + Keyboard (this project)** | [Article](https://controllerstech.com/stm32-usb-hid-mouse-keyboard/) |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://ko-fi.com/controllerstech).
