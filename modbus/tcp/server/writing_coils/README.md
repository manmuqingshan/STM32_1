# STM32 Modbus TCP Server — Writing Coils

Part 5 of the STM32 Modbus TCP series. This project builds on the Part 4 server and adds function code 5 (Write Single Coil) and function code 15 (Write Multiple Coils). Both function codes write to the same 20-coil database introduced in Part 4, and this is also the final part covering the STM32 as a Modbus server — every function code the server needs to support is now in place.

## 📺 Video Tutorial

[STM32 Modbus TCP Server using LWIP — Writing Coils | Modbus TCP Series #5](https://youtu.be/Pt4LEG4q5f8)

## 📖 Full Article

[STM32 Modbus TCP Server using LWIP – Part 5: Writing Coils](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part5-write-coils/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32 Nucleo H755ZI |
| Ethernet | Onboard RMII PHY (LAN8742) |
| UART | UART3 — ST-Link virtual COM port |
| ADC Input | 10k potentiometer, wiper on ADC1 channel 15 |
| Discrete Inputs | 5 wires wired to common ground, internal pull-up enabled |
| Coil Outputs | 5 LEDs wired to GPIO output pins |
| Network | Direct Ethernet connection or router (static IP) |

### Pin Assignment

| Signal | Pin | Function |
|--------|-----|----------|
| Ethernet RMII | Auto-assigned | Configured via CubeMX Connectivity → Ethernet, mode RMII |
| UART3 TX | PD8 | ST-Link virtual COM port TX |
| UART3 RX | PD9 | ST-Link virtual COM port RX |
| ADC1 Channel 15 | PA3 | Potentiometer wiper input |
| Input1 | PF14 | Discrete input 1, pull-up enabled |
| Input2 | PF15 | Discrete input 2, pull-up enabled |
| Input3 | PD0 | Discrete input 3, pull-up enabled |
| Input4 | PD1 | Discrete input 4, pull-up enabled |
| Input5 | PB14 | Discrete input 5, pull-up enabled |
| Coil1 | PE10 | LED output, coil 1 |
| Coil2 | PE12 | LED output, coil 2 |
| Coil3 | PE15 | LED output, coil 3 |
| Coil4 | PB10 | LED output, coil 4 |
| Coil5 | PB11 | LED output, coil 5 |

> The potentiometer, the five switches, and their wiring stay exactly as they were in Part 4. Five new wires are added for the LEDs, representing the first five coils in the database.

---

## What This Project Does

- Continues the Ethernet, LWIP, ADC, discrete input, and MPU configuration from Part 4 — unchanged in this project
- Adds five GPIO output pins in CubeMX, one for each LED, with GPIO context assigned to Cortex-M7
- Adds the `MB_updateCoils` function, which drives the five LED pins based on the first five bits of the existing `MB_Coils` database
- Handles function code 5, writing a single coil based on a client-supplied `0xFF00` (set) or `0x0000` (reset) value
- Handles function code 15, writing multiple coils in one request by unpacking the client's byte-packed coil data bit by bit
- Validates every write request: address range against `MB_COIL_COUNT`, value against the `0xFF00`/`0x0000` convention for FC05, and quantity (max 1968) plus byte count against the requested coils for FC15, returning Illegal Data Value or Illegal Data Address exceptions when a request is invalid
- Completes every function code planned for the server side of this series — reading and writing holding registers, reading input registers, reading and writing coils, and reading discrete inputs

---

## How It Works

Function code 5 pulls the coil address and the 16-bit value out of the request data, and validates the address against `MB_COIL_COUNT`. It then works out which byte and bit inside `MB_Coils` represent that coil. If the value is `0xFF00`, the bit is set; if it is `0x0000`, the bit is cleared; any other value returns an Illegal Data Value exception. The response simply echoes the function code, address, and value back to the client.

Function code 15 pulls the starting address, quantity, and byte count out of the request, and runs three checks: the quantity must be between 1 and 1968, the byte count must match the quantity rounded up to the nearest byte, and the address range must fit inside `MB_COIL_COUNT`. Once validated, it loops once per coil, extracting the bit for that coil from the client's data and writing it into the matching byte and bit inside `MB_Coils`. The response echoes back the function code, starting address, and quantity written.

Both functions reuse the same MBAP header pattern used throughout the series, with the length field fixed at 6 bytes, since neither response needs to carry variable-length data back to the client.

### Key Files

| File | What it does |
|------|--------------|
| `modbus.h` | Function code definitions, exception codes, and the `MB_Request_t` structure — unchanged from Part 1 |
| `Modbus_Database.h` | Holding register, input register, coil, and discrete input array declarations — unchanged from Part 4 |
| `Modbus_Database.c` | Adds `MB_updateCoils`, which drives the five LED pins from the first five bits of `MB_Coils` |
| `Modbus_Server.c` | Sets up the TCP server — unchanged from Part 1 |
| `Modbus_Parser.c` | Adds `MB_FC05_WriteSingleCoil` and `MB_FC15_WriteMultipleCoils`, and routes both from `MB_ProcessFunction` |
| `main.c` | Calls `MB_updateCoils` inside the infinite loop, alongside `MB_UpdateDiscreteInputs` |

---

## Project Structure

```
modbus/tcp/server/write_coils/
├── CM7/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── main.h
│   │   └── Src/
│   │       └── main.c              ← MB_updateCoils call added
│   └── Drivers/
│       └── Modbus/
│           ├── modbus.h
│           ├── Modbus_Database.h
│           ├── Modbus_Database.c    ← MB_updateCoils added
│           ├── Modbus_Server.c
│           ├── Modbus_Server.h
│           ├── Modbus_Parser.c      ← FC05 and FC15 handlers added here
│           └── Modbus_Parser.h
├── LWIP/
│   └── Target/
│       └── ethernetif.c            ← RX/TX descriptor and RX pool sections
└── .ioc                            ← STM32CubeMX project file
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
| ADC1 | Channel 15 (PA3), 16-bit resolution, blocking mode, clock source: peripheral clock (64 MHz) |
| GPIO Input1–Input5 | Input mode, pull-up enabled, GPIO context: Cortex-M7 |
| GPIO Coil1–Coil5 | Output mode, GPIO context: Cortex-M7 |

**New in this part:** the five coil output pins (Coil1 through Coil5). Everything else above is carried over from Part 4 as-is.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. Open the generated project in STM32CubeIDE
4. Add the section relocation for `.RxDescripSection`, `.TxDescripSection`, and `.Rx_PoolSection` to the linker script, placing them in `RAM_D2`
5. Wire a 10k potentiometer — outer pins to 3.3V and GND, wiper to PA3
6. Wire five switches (or jumper wires) from PF14, PF15, PD0, PD1, and PB14 to a common ground
7. Wire five LEDs (with current-limiting resistors) from PE10, PE12, PE15, PB10, and PB11 to ground
8. Build and flash to the board
9. Set your PC's Ethernet adapter to the same subnet as the board if connecting directly

---

## Testing

Open a serial monitor on the ST-Link virtual COM port at **115200 baud, 8N1**. On reset, the console prints:
```
Initializing Modbus TCP Server...
Port : 502
Server is Listening...
```

Connect using a Modbus TCP client (for example, [Simply Modbus TCP Client](https://simplymodbus.ca/tools-client.html)) using the board's IP address and port 502.

**Function code 5 — Write Single Coil:** select coil 1, currently reset, and send the value `0xFF00`. The first LED turns on, confirming the coil was set. Sending `0x0000` to the same coil turns the LED back off. Sending any value other than `0xFF00` or `0x0000` returns an Illegal Data Value exception.

**Function code 15 — Write Multiple Coils:** write to five coils starting from coil 1, with all values set to 0. All five LEDs turn off in one request. Setting only coil 2 and coil 4 while keeping the rest at 0 turns on only the second and fourth LEDs, matching the returned data on a follow-up read.

**Out-of-range coil write:** attempt to write a coil address at or beyond `MB_COIL_COUNT`. Since only 20 coils are defined, the server responds with an Illegal Data Address exception:
```
========== Modbus TCP Request ==========
Transaction ID : 1
Protocol ID    : 0
Length         : 6
Unit ID        : 9
Function Code  : 0x0F
Sending Exception Response
Exception Code : 0x02
```

**Byte count mismatch (FC15):** send a Write Multiple Coils request where the byte count does not match the quantity of coils requested. The server rejects it with an Illegal Data Value exception, without modifying `MB_Coils`.

**Full function code sweep:** with the server running, read and write holding registers (FC03/FC06), read input registers (FC04), read and write coils (FC01/FC05/FC15), and read discrete inputs (FC02) in sequence. Every request returns the expected data, confirming the server now handles all eight function codes implemented across this series.

This confirms both write function codes update the coil database correctly, drive the LEDs as expected, and that the server rejects any write request that falls outside the defined coil range or violates the FC05/FC15 value rules.

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | Ethernet Setup and Basic Server | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part1/) |
| Part 2 | Reading Holding & Input Registers | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part2-registers/) |
| Part 3 | Writing Holding Registers | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part3-write-registers/) |
| Part 4 | Reading Coils & Discrete Inputs | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part4-read-coils-discrete-inputs/) |
| **Part 5** | **Writing Coils (this project)** | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part5-write-coils/) |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
