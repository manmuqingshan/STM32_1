# STM32 Modbus TCP Client — Writing Registers and Coils

Part 8 of the STM32 Modbus TCP series, continuing on the client side. [Part 7](https://controllerstech.com/stm32-modbus-tcp-client-lwip-part7-reading-registers-coils/) extended the STM32 Modbus client to read holding registers, input registers, coils, and discrete inputs from the server, including exception handling for out-of-range requests. This project builds on that same client and adds the ability to modify data on the server: writing a single register, multiple registers, a single coil, and multiple coils. A push button on the Nucleo board triggers each write in turn, and the client decodes the server's confirmation for every request.

## 📺 Video Tutorial

[STM32 Modbus TCP Client using LWIP — Writing Registers and Coils | Modbus TCP Series #8](https://youtu.be/DwunDU8U8Ik)

## 📖 Full Article

[STM32 Modbus TCP Client using LWIP – Part 8: Writing Registers and Coils](https://controllerstech.com/stm32-modbus-tcp-client-lwip-part8-writing-registers-coils/)

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
| User Button | PC13 | GPIO external interrupt, rising edge, triggers the next Modbus write or read request |

> PC13 is already pulled down on the Nucleo board, so no internal pull-up or pull-down is configured. Since this is a dual-core board, the pin context assignment for PC13 must be set to Cortex-M7, as the entire application runs on that core.

---

## What This Project Does

- Adds four write functions to `Modbus_Client_Request.c` — `MB_WriteSingleRegister`, `MB_WriteMultipleRegisters`, `MB_WriteSingleCoil`, and `MB_WriteMultipleCoils`
- Calculates a variable length and byte count for the multiple-write functions, since the frame size now depends on how much data is being written
- Packs individual coil values into bits, eight per byte, inside `MB_WriteMultipleCoils`, the same way the server unpacks them when reading coils
- Updates `Modbus_Client_Parser.c` with four new handler functions that decode the server's write confirmation and print the address, value, or quantity that was written
- Extends the `counter` switch statement inside `main.c` to six cases — four writes followed by two reads, so the client can verify its own writes against the server
- Reuses the existing `MB_CurrentRequest` structure, TCP connection handling, and reconnect logic from Part 6 and Part 7 without any changes

---

## How It Works

The button handling stays identical to Part 7. Every press sets the `isPressed` flag inside `HAL_GPIO_EXTI_Callback`, and the main loop debounces, increments the counter, and calls the matching function for that case.

For a single write, the request is a fixed 12 bytes: the MBAP header followed by the function code, address, and value. For a multiple write, the byte count is calculated first, since it decides both the length field in the MBAP header and how many bytes of actual register or coil data follow the byte count field in the PDU. Register data is copied two bytes at a time. Coil data is packed bit by bit using nested loops, an outer loop for each byte and an inner loop for each of the eight bits inside it, with any unused bits in the last byte left at zero.

Once the server processes the request, it sends a short confirmation back. `MB_Client_Parser` reads the function code at index 7 and calls the matching handler. Single write confirmations echo back the address and value, and `MB_ParseWriteSingleCoil` translates the value into ON or OFF based on whether it received `0xFF00` or `0x0000`. Multiple write confirmations only echo back the start address and quantity, since the server does not need to repeat the actual data.

The last two cases in the counter switch are read requests, not writes. These call `MB_ReadHoldingRegisters` and `MB_ReadCoils` from Part 7, so the same project can confirm that the values it just wrote are actually present on the server.

### Key Files

| File | What it does |
|------|--------------|
| `modbus.h` | Function code definitions and exception codes — shared with the server side of this series |
| `Modbus_Client.h` / `Modbus_Client.c` | TCP client setup and callbacks — unchanged since Part 6 |
| `Modbus_Client_Request.h` / `Modbus_Client_Request.c` | Builds the request frame for each write function code, alongside the read functions carried over from Part 7 |
| `Modbus_Client_Parser.h` / `Modbus_Client_Parser.c` | Decodes write confirmations for single/multiple registers and coils, in addition to the read parsers and exception handling from Part 7 |
| `main.c` | Reads the user button on PC13 and cycles through four write requests followed by two read requests on every press |

---

## Project Structure

```
modbus/tcp/client/write_registers_coils/
├── CM7/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── main.h
│   │   └── Src/
│   │       └── main.c              ← six-case switch: writes + read-back added
│   └── Drivers/
│       └── Modbus/
│           ├── modbus.h
│           ├── Modbus_Client.h
│           ├── Modbus_Client.c
│           ├── Modbus_Client_Request.h     ← write functions added
│           ├── Modbus_Client_Request.c     ← write functions added
│           ├── Modbus_Client_Parser.h
│           └── Modbus_Client_Parser.c      ← write confirmation handlers added
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

**This continues the Part 7 project** — no Ethernet, LWIP, MPU, or GPIO changes are needed here. Every setting above is identical to what was already configured in Parts 6 and 7.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. Open the generated project in STM32CubeIDE
4. Add the section relocation for `.RxDescripSection`, `.TxDescripSection`, and `.Rx_PoolSection` to the linker script, placing them in `RAM_D2`, same as Part 6
5. Overwrite the existing `Modbus_Client_Request.c`, `Modbus_Client_Request.h`, and `Modbus_Client_Parser.c` in the Modbus driver folder with the versions from this project
6. Inside `main.c`, update the IP address and port passed to `Modbus_Client_Process`, and adjust the register/coil addresses, values, and array contents in each write call to match your server's database
7. Update the counter reset condition to `if (counter >= 6) counter = 0;`, since two more cases have been added
8. Build and flash to the board
9. Run a Modbus TCP server on your PC — a Python script, a Modbus simulator, or the STM32 server project from Parts 1–5 — listening on port 502, with a known database size for registers and coils

---

## Testing

Open a serial monitor on the ST-Link virtual COM port at **115200 baud, 8N1**. Once the client connects, press the user button (PC13) to trigger each write in sequence, followed by the two read-back requests.

**Writing a single register:**
```
Write Single Register Response
-------------------------------
Address : 5
Value   : 8888
```

**Writing multiple registers:**
```
Write Multiple Registers Response
---------------------------------
Start Address : 7
Quantity      : 3
```

**Writing a single coil:**
```
Write Single Coil Response
---------------------------
Address : 5
Value   : OFF
```

**Writing multiple coils:**
```
Write Multiple Coils Response
-----------------------------
Start Address : 10
Quantity      : 6
```

**Read-back confirmation (holding registers and coils):** the fifth and sixth button presses call `MB_ReadHoldingRegisters` and `MB_ReadCoils` from Part 7, printing the updated values so you can confirm they match what was just written. Restarting the Python server resets its database, so writing the same values again will show a clear change from the original data.

If a request times out or the connection drops, the client falls back to the same reconnect and retry behaviour built in Part 6.

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | Ethernet Setup and Basic Server | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part1/) |
| Part 2 | Modbus TCP Server — Reading Holding & Input Registers | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part2-registers/) |
| Part 3 | Modbus TCP Server — Writing Holding Registers | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part3-write-registers/) |
| Part 4 | Modbus TCP Server — Reading Coils & Discrete Inputs | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part4-read-coils-discrete-inputs/) |
| Part 5 | Modbus TCP Server — Writing Coils | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part5-write-coils/) |
| Part 6 | Modbus TCP Client — Basic Setup | [Article](https://controllerstech.com/stm32-modbus-tcp-client-lwip-part6-client-setup/) |
| Part 7 | Modbus TCP Client — Reading Registers and Coils | [Article](https://controllerstech.com/stm32-modbus-tcp-client-lwip-part7-reading-registers-coils/) |
| **Part 8** | **Modbus TCP Client — Writing Registers and Coils (this project)** | [Article](https://controllerstech.com/stm32-modbus-tcp-client-lwip-part8-writing-registers-coils/) |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
