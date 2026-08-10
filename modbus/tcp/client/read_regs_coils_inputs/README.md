# STM32 Modbus TCP Client — Reading Registers and Coils

Part 7 of the STM32 Modbus TCP series, continuing on the client side. [Part 6](https://controllerstech.com/stm32-modbus-tcp-client-lwip-part6-client-setup/) configured the STM32 as a basic Modbus TCP client — Ethernet, LWIP, and a single test request, with no function code handling. This project builds on that same client and adds the ability to actually read data from the server: holding registers, input registers, coils, and discrete inputs. A push button on the Nucleo board triggers each read in turn, and the client decodes the response, or the exception, that comes back.

## 📺 Video Tutorial

[STM32 Modbus TCP Client using LWIP — Reading Registers and Coils | Modbus TCP Series #7](https://youtu.be/e5sEJCo8AuE)

## 📖 Full Article

[STM32 Modbus TCP Client using LWIP – Part 7: Reading Registers and Coils](https://controllerstech.com/stm32-modbus-tcp-client-lwip-part7-reading-registers-coils/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32 Nucleo H755ZI |
| Ethernet | Onboard RMII PHY (LAN8742) |
| UART | UART3 — ST-Link virtual COM port |
| User Input | Onboard user button, PC13 |
| Modbus Server | Any Modbus TCP server on port 502 — a PC running a Python script, a Modbus simulator, or the STM32 server built in Parts 1–5 |
| Network | Direct Ethernet connection or router (static IP) |

### Pin Assignment

| Signal | Pin | Function |
|--------|-----|----------|
| Ethernet RMII | Auto-assigned | Configured via CubeMX Connectivity → Ethernet, mode RMII |
| UART3 TX | PD8 | ST-Link virtual COM port TX |
| UART3 RX | PD9 | ST-Link virtual COM port RX |
| User Button | PC13 | GPIO external interrupt, rising edge, triggers the next Modbus read request |

> PC13 is already pulled down on the Nucleo board, so no internal pull-up or pull-down is configured. Since this is a dual-core board, the pin context assignment for PC13 must be set to Cortex-M7, as the entire application runs on that core.

---

## What This Project Does

- Adds `Modbus_Client_Request.c`, with a request-building function for each of the four read function codes — `MB_ReadHoldingRegisters`, `MB_ReadInputRegisters`, `MB_ReadCoils`, and `MB_ReadDiscreteInputs`
- Stores the requested start address and quantity in a `MB_CurrentRequest` structure before sending, since the server's response does not repeat the address back
- Updates `Modbus_Client_Parser.c` to fully decode each response — copying register values into `HoldingRegBuffer` / `InputRegBuffer`, and unpacking individual coil and discrete input bits into `CoilBuffer` / `InputBuffer`
- Adds exception handling, recognizing the MSB-set function code in the response and printing the matching exception code, such as Illegal Data Address
- Configures PC13 as a GPIO external interrupt, wired to the onboard user button
- Uses a `counter` variable inside `main.c` to cycle through the four read functions on every button press, with a debounce delay in between

---

## How It Works

Every button press sets a volatile `isPressed` flag inside `HAL_GPIO_EXTI_Callback`. The main loop checks this flag, waits briefly for debouncing, increments a counter, and calls the matching read function — holding registers on the first press, input registers on the second, coils on the third, and discrete inputs on the fourth, before wrapping back to the start.

Each read function builds a 12-byte Modbus TCP request, the MBAP header followed by the PDU, and saves the requested start address and quantity in `MB_CurrentRequest`. This matters because the server's response only contains a byte count and the data itself, never the address, so the client needs to remember it to place the incoming values correctly.

When the server responds, `MB_Client_Parser` reads the function code at index 7. If the MSB is set, the response is an exception, and `MB_ParseException` prints the function code and exception code involved. Otherwise, the matching parser function runs: register-based responses combine two bytes per value, while coil and discrete input responses unpack each byte into eight individual bits. Every parser prints its decoded values on the serial console.

### Key Files

| File | What it does |
|------|--------------|
| `modbus.h` | Function code definitions and exception codes — shared with the server side of this series |
| `Modbus_Client.h` / `Modbus_Client.c` | TCP client setup and callbacks — unchanged since Part 6 |
| `Modbus_Client_Request.h` / `Modbus_Client_Request.c` | Builds the request frame for each read function code, and stores the requested address and quantity |
| `Modbus_Client_Parser.h` / `Modbus_Client_Parser.c` | Decodes holding registers, input registers, coils, and discrete inputs from the server's response, and handles exception responses |
| `main.c` | Reads the user button on PC13 and triggers the next read function on every press |

---

## Project Structure

```
modbus/tcp/client/read_registers_coils/
├── CM7/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── main.h
│   │   └── Src/
│   │       └── main.c              ← button handling and read function calls added
│   └── Drivers/
│       └── Modbus/
│           ├── modbus.h
│           ├── Modbus_Client.h
│           ├── Modbus_Client.c
│           ├── Modbus_Client_Request.h     ← new
│           ├── Modbus_Client_Request.c     ← new
│           ├── Modbus_Client_Parser.h
│           └── Modbus_Client_Parser.c      ← updated with full decoding
├── LWIP/
│   └── Target/
│       └── ethernetif.c            ← RX/TX descriptor and RX pool sections
└── .ioc                             ← STM32CubeMX project file
```

---

## CubeMX Configuration

| Peripheral | Setting |
|------------|---------|
| Clock | HSI 64 MHz → PLL → 400 MHz system clock |
| Ethernet | RMII mode, Cortex-M7 core only |
| LWIP | Static IP `192.168.1.100`, subnet `255.255.255.0`, gateway `192.168.1.1` |
| LWIP Heap | 10 KB, starting at `0x30004900` |
| PHY | LAN8742 |
| MPU Region | `0x30000000`, 32 KB, TEX level 1, non-cacheable, non-bufferable |
| UART3 | 115200 baud, 8N1, TX → PD8, RX → PD9 |
| GPIO (User Button) | PC13, GPIO_MODE_IT_RISING, no pull, pin context set to Cortex-M7, EXTI15_10 interrupt enabled in NVIC |

**This continues the Part 6 project** — no Ethernet, LWIP, or MPU changes are needed here beyond what was already configured. The only addition in CubeMX is the PC13 external interrupt for the user button.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. Open the generated project in STM32CubeIDE
4. Add the section relocation for `.RxDescripSection`, `.TxDescripSection`, and `.Rx_PoolSection` to the linker script, placing them in `RAM_D2`, same as Part 6
5. Add `Modbus_Client_Request.c` and `Modbus_Client_Request.h` to the Modbus driver folder, and overwrite the existing `Modbus_Client_Parser.c` with the updated version
6. Include `Modbus_Client_Request.h` in `main.c`
7. Inside `main.c`, update the IP address and port passed to `Modbus_Client_Process`, and adjust the start address and quantity in each read call to match your server's database
8. Build and flash to the board
9. Run a Modbus TCP server on your PC — a Python script, a Modbus simulator, or the STM32 server project from Parts 1–5 — listening on port 502, with a known database size for registers, coils, and discrete inputs

---

## Testing

Open a serial monitor on the ST-Link virtual COM port at **115200 baud, 8N1**. Once the client connects, press the user button (PC13) to trigger each read in sequence.

**Holding registers (valid range):**
```
[000] = 120
[001] = 45
...
[009] = 78
```

**Holding registers (address beyond the server's database):**
```
Modbus Exception Response
-------------------------
Function Code : 3
Exception Code: 2 - Illegal Data Address
```

**Input registers, within range:** prints the decoded 16-bit values the same way as holding registers, stored in `InputRegBuffer`.

**Coils (address beyond the server's database):** returns the same Illegal Data Address exception, with function code 1.

**Discrete inputs, within range:** prints one line per bit, including any padding bits the server adds to complete the last byte. For example, requesting 5 discrete inputs still returns 8 bits, with the last 3 padded as zero.

If a request times out or the connection drops, the client falls back to the same reconnect and retry behaviour built in Part 6.

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | Ethernet Setup and Basic Server | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part1/) |
| Part 2 | Reading Holding & Input Registers | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part2-registers/) |
| Part 3 | Writing Holding Registers | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part3-write-registers/) |
| Part 4 | Reading Coils & Discrete Inputs | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part4-read-coils-discrete-inputs/) |
| Part 5 | Writing Coils | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part5-write-coils/) |
| Part 6 | Modbus TCP Client — Basic Setup | [Article](https://controllerstech.com/stm32-modbus-tcp-client-lwip-part6-client-setup/) |
| **Part 7** | **Modbus TCP Client — Reading Registers and Coils (this project)** | [Article](https://controllerstech.com/stm32-modbus-tcp-client-lwip-part7-reading-registers-coils/) |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
