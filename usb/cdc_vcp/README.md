# STM32 USB CDC — Virtual COM Port

Part 1 of the STM32 USB series. This project implements USB CDC (Communication Device Class) on an STM32F446 board using STM32CubeMX and HAL. The STM32 appears as a Virtual COM Port on the host computer — no USB-to-UART bridge chip needed. We transmit data, receive commands to control an onboard LED, and redirect `printf` over USB.

## 📺 Video Tutorial

[STM32 USB CDC — Virtual COM Port, Transmit & Receive Data | USB Series #1](https://youtu.be/O4o-ed5veh4)

## 📖 Full Article

[STM32 USB CDC Virtual COM Port: CubeMX Setup, Transmit & Receive Data](https://controllerstech.com/stm32-usb-cdc-virtual-com-port/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32F446 — weAct Studio |
| USB Connector | USB-C — data lines on PA11 (D−) and PA12 (D+) |
| LED | Connected to PB2 — GPIO output |
| Crystal | 8 MHz — used to derive 48 MHz for USB peripheral |

---

## What This Project Does

- Configures USB OTG FS peripheral in Device Only mode using STM32CubeMX
- Makes the STM32 appear as a Virtual COM Port on the host computer
- Transmits "Hello World" over USB every one second
- Receives "ON" / "OFF" commands from the PC and controls the onboard LED
- Redirects `printf` output over USB CDC using a custom `_write` syscall override

---

## Function / API Used

| Function | File | Description |
|----------|------|-------------|
| `CDC_Transmit_FS()` | `usbd_cdc_if.c` | Transmit data from STM32 to host |
| `CDC_Receive_FS()` | `usbd_cdc_if.c` | Called automatically when host sends data |
| `_write()` | `main.c` | Overrides syscall to redirect printf over USB |

---

## Project Structure

```
usb/cdc_vcp/
├── Core/
│   ├── Inc/
│   │   └── main.h
│   └── Src/
│       └── main.c              ← Transmit, receive, LED control, printf redirect
├── USB_DEVICE/
│   ├── App/
│   │   ├── usbd_cdc_if.c       ← CDC receive handler, recvDone flag, recvLen
│   │   └── usbd_cdc_if.h
│   └── Target/
│       └── usbd_conf.c
├── Drivers/
└── .ioc                        ← STM32CubeMX project file
```

---

## CubeMX Configuration

| Peripheral | Pin | Mode |
|------------|-----|------|
| USB OTG FS | PA11, PA12 | Device Only |
| GPIO Output | PB2 | Onboard LED |
| HSE Crystal | — | 8 MHz external crystal |
| PLL | — | System clock 180 MHz, USB clock 48 MHz |

**USB clock note:** The USB peripheral requires exactly 48 MHz. After enabling USB OTG FS, go to the Clock Configuration tab and adjust the PLL multipliers/dividers so that the 48 MHz USB clock is derived correctly while keeping the system clock at 180 MHz.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. Open the generated project in STM32CubeIDE
4. Build and flash to the board
5. Connect the USB cable to the board's USB-C connector
6. Open any serial monitor — the device appears as a Virtual COM Port

---

## Testing

**Transmit test:** Open the serial monitor after connecting. "Hello World" should print every one second. The baud rate setting does not matter — USB CDC ignores it.

**Receive test:** Send the string `ON` from the serial monitor. The onboard LED turns on and the board responds with `LED Turned ON`. Send `OFF` to turn it off.

**printf test:** With the `_write` override in place, any `printf()` call in the project outputs to the serial monitor over USB.

---

## USB Reconnect Behavior

| Board | Behavior after reflash |
|-------|------------------------|
| STM32F446 (weAct) | Device reappears automatically — no replug needed |
| Blue Pill | Must physically unplug and replug USB cable after reflash |

This difference is due to how each board reinitializes the USB peripheral after a firmware reset.

---

## Series

| Part | Topic | Link |
|------|-------|------|
| **Part 1** | **CDC — Virtual COM Port (this project)** | [Article](https://controllerstech.com/stm32-usb-cdc-virtual-com-port/) |
| Part 2 | Coming soon | — |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://ko-fi.com/controllerstech).
