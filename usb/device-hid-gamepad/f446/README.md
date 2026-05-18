# STM32 USB HID — Gamepad / Joystick

Part 2 of the STM32 USB series. This project configures an STM32 as a USB HID gamepad using STM32CubeMX and HAL. A custom HID report descriptor replaces the default mouse descriptor, an analog joystick is read over ADC with DMA, and reports are sent to the host every 10 ms. No custom drivers are needed on the PC side — Windows, macOS, and any modern browser recognise the device automatically.

Two hardware targets are covered:

| Directory | Board | ADC Resolution | Notes |
|-----------|-------|----------------|-------|
| `f446/` | STM32F446 — weAct Studio | 8-bit (native) | Values sent directly to HID buffer |
| `f103/` | STM32F103 — Blue Pill | 12-bit → shifted to 8-bit | `ADC_Val >> 4` before filling HID buffer |

---

## 📺 Video Tutorial

[STM32 USB HID Gamepad — Configure STM32 as a Game Controller | USB Series #2](https://youtu.be/rWdHumNaP8A)

## 📖 Full Article

[STM32 USB HID Gamepad: Configure STM32 as a Game Controller](https://controllerstech.com/stm32-usb-hid-gamepad/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board (primary) | STM32F446 — weAct Studio |
| Board (secondary) | STM32F103 — Blue Pill |
| Joystick Module | Analog joystick — VRx, VRy, SW pins |
| VRx | PA0 — ADC1 Channel 0 (X axis) |
| VRy | PA1 — ADC1 Channel 1 (Y axis) |
| SW (button) | PA2 — GPIO Input with internal pull-up |

---

## What This Project Does

- Configures USB OTG FS in Device Only mode using STM32CubeMX
- Replaces the default mouse HID report descriptor with a custom gamepad descriptor
- Reads X and Y axis values from an analog joystick over ADC with DMA in circular mode
- Reads a push button on PA2 using an internal pull-up
- Sends a 3-byte HID report (X, Y, button) to the host every 10 ms
- Works in any gamepad tester (hardwaretester.com) and as a controller in browser-based retro emulators — no drivers required

---

## HID Report Descriptor

The default mouse descriptor in `usbd_hid.c` is replaced with the following gamepad descriptor (43 bytes):

```c
__ALIGN_BEGIN static uint8_t HID_MOUSE_ReportDesc[HID_MOUSE_REPORT_DESC_SIZE] __ALIGN_END =
{
  0x05, 0x01,  /* Usage Page (Generic Desktop Ctrls)  */
  0x09, 0x05,  /* Usage (Gamepad)                     */
  0xA1, 0x01,  /* Collection (Application)            */
  /* X and Y axes */
  0x09, 0x30,  /*   Usage (X)                         */
  0x09, 0x31,  /*   Usage (Y)                         */
  0x15, 0x00,  /*   Logical Minimum (0)               */
  0x25, 0xFF,  /*   Logical Maximum (255)             */
  0x75, 0x08,  /*   Report Size (8 bits)              */
  0x95, 0x02,  /*   Report Count (2)                  */
  0x81, 0x02,  /*   Input (Data, Var, Abs)            */
  /* Button */
  0x05, 0x09,  /*   Usage Page (Button)               */
  0x19, 0x01,  /*   Usage Minimum (0x01)              */
  0x29, 0x01,  /*   Usage Maximum (0x01)              */
  0x15, 0x00,  /*   Logical Minimum (0)               */
  0x25, 0x01,  /*   Logical Maximum (1)               */
  0x95, 0x01,  /*   Report Count (1)                  */
  0x75, 0x01,  /*   Report Size (1 bit)               */
  0x81, 0x02,  /*   Input (Data, Var, Abs)            */
  /* Padding — 7 bits to complete the byte */
  0x95, 0x01,  /*   Report Count (1)                  */
  0x75, 0x07,  /*   Report Size (7 bits)              */
  0x81, 0x01,  /*   Input (Const, Array, Abs)         */
  0xC0         /* End Collection                      */
};
```

Also update `usbd_hid.h` to match:

```c
#define HID_MOUSE_REPORT_DESC_SIZE    43U
#define HID_EPIN_SIZE                 0x03U
```

---

## Functions / API Used

| Function | File | Description |
|----------|------|-------------|
| `USBD_HID_SendReport()` | `usbd_hid.c` | Send a HID report from STM32 to host |
| `HAL_ADC_Start_DMA()` | `main.c` | Start ADC conversions with DMA in circular mode |
| `HAL_ADC_ConvCpltCallback()` | `main.c` | Callback fires after both channels convert — copies ADC values into HID buffer |
| `HAL_GPIO_ReadPin()` | `main.c` | Reads PA2 button state |

---

## Project Structure

```
usb/hid_gamepad/
├── f446/                           ← STM32F446 — weAct Studio (8-bit ADC, native)
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── main.h
│   │   └── Src/
│   │       └── main.c              ← ADC DMA callback, button read, HID send loop
│   ├── USB_DEVICE/
│   │   ├── App/
│   │   │   └── usbd_hid.c          ← Modified: gamepad report descriptor
│   │   └── Target/
│   │       └── usbd_conf.c
│   ├── Middlewares/
│   │   └── ST/STM32_USB_Device_Library/Class/HID/Src/
│   │       └── usbd_hid.h          ← Modified: DESC_SIZE = 43, EPIN_SIZE = 0x03
│   ├── Drivers/
│   └── .ioc                        ← STM32CubeMX project file
│
└── f103/                           ← STM32F103 — Blue Pill (12-bit ADC, shifted)
    ├── Core/
    │   ├── Inc/
    │   │   └── main.h
    │   └── Src/
    │       └── main.c              ← Same logic; ADC_Val >> 4 before HID buffer fill
    ├── USB_DEVICE/
    │   ├── App/
    │   │   └── usbd_hid.c          ← Same gamepad descriptor as f446
    │   └── Target/
    │       └── usbd_conf.c
    ├── Middlewares/
    │   └── ST/STM32_USB_Device_Library/Class/HID/Src/
    │       └── usbd_hid.h          ← Modified: DESC_SIZE = 43, EPIN_SIZE = 0x03
    ├── Drivers/
    └── .ioc
```

---

## CubeMX Configuration

### Common (both boards)

| Peripheral | Pin | Mode |
|------------|-----|------|
| USB OTG FS | PA11, PA12 | Device Only |
| ADC1 Ch0 | PA0 | X axis (VRx) |
| ADC1 Ch1 | PA1 | Y axis (VRy) |
| GPIO Input | PA2 | Button (SW) — Pull-up enabled |

### STM32F446 — `f446/`

| Setting | Value |
|---------|-------|
| ADC Resolution | 8-bit |
| DMA Mode | Circular, Byte width |
| Scan Conversion | Enabled |
| Continuous Conversion | Enabled |
| DMA Continuous Requests | Enabled |
| End of Conversion Selection | End of All Conversions |
| System Clock | 180 MHz |
| USB Clock | 48 MHz |

### STM32F103 — `f103/`

| Setting | Value |
|---------|-------|
| ADC Resolution | 12-bit (hardware fixed) |
| ADC shift in code | `ADC_Val[n] >> 4` maps 0–4095 → 0–255 |
| DMA Mode | Circular, Half-word width |
| System Clock | 72 MHz |
| USB Clock | 48 MHz (derived via PLL) |

> **USB clock note:** The USB peripheral requires exactly 48 MHz on both boards. Verify this in the Clock Configuration tab before generating code.

---

## Key Difference: F446 vs F103

The only meaningful code difference between the two directories is how ADC values are placed into the HID buffer.

**F446** — 8-bit ADC, values already in 0–255 range, used directly:

```c
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    HID_Buffer[0] = ADC_Val[0];   // X axis — 8-bit, no shift needed
    HID_Buffer[1] = ADC_Val[1];   // Y axis — 8-bit, no shift needed
}
```

**F103** — 12-bit ADC, shifted right by 4 before filling the buffer:

```c
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    HID_Buffer[0] = ADC_Val[0] >> 4;  // 0–4095 → 0–255
    HID_Buffer[1] = ADC_Val[1] >> 4;  // 0–4095 → 0–255
}
```

The HID descriptor, report size, send loop, and button logic are identical in both directories.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file for your target board in STM32CubeMX and generate code
3. **Re-apply descriptor changes** — paste the gamepad descriptor back into `usbd_hid.c` and update the two defines in `usbd_hid.h` (CubeMX overwrites these files on regeneration)
4. Open the generated project in STM32CubeIDE
5. Build and flash to the board
6. Connect the USB cable to the board's USB connector

> **Blue Pill note:** After reflashing, you must physically unplug and replug the USB cable. The F103 does not reinitialise the USB peripheral automatically on firmware reset the way the F446 does.

---

## Testing

**Gamepad Tester:** Open [hardwaretester.com/gamepad](https://hardwaretester.com/gamepad). The device will appear as **STM32 Gamepad**. Move the joystick — both axes should respond in real time. Press the button — the button indicator should light up.

**Windows Device Manager:** Look under Human Interface Devices — STM32 Gamepad should be listed after connecting.

**macOS:** Go to Settings → General → System Report → USB to confirm enumeration.

**Game controller:** Open [retrogames.cc](https://www.retrogames.cc/) or any browser-based emulator that supports gamepad input. The joystick axes map to movement and the button maps to an action — no configuration required.

---

## USB Reconnect Behaviour

| Board | Behaviour after reflash |
|-------|------------------------|
| STM32F446 (weAct) | Device reappears automatically — no replug needed |
| STM32F103 (Blue Pill) | Must physically unplug and replug USB cable after reflash |

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | CDC — Virtual COM Port | [Article](https://controllerstech.com/stm32-usb-cdc-virtual-com-port/) |
| **Part 2** | **HID — Gamepad / Joystick (this project)** | [Article](https://controllerstech.com/stm32-usb-hid-gamepad/) |
| Part 3 | Coming soon | — |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://ko-fi.com/controllerstech).
