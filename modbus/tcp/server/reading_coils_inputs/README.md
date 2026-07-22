# STM32 Modbus TCP Server — Reading Coils and Discrete Inputs

Part 4 of the STM32 Modbus TCP series. This project builds on the Part 3 server and adds function code 1 (Read Coils) and function code 2 (Read Discrete Inputs). Coils are a new 1-bit, read/write database, and discrete inputs are a new 1-bit, read-only database fed from five hardware input pins. Writing to coils is not covered here — that is handled in Part 5, using function codes 5 and 15.

## 📺 Video Tutorial

[STM32 Modbus TCP Server using LWIP — Reading Coils and Discrete Inputs | Modbus TCP Series #4](https://youtu.be/uQSSrSXbBf8)

## 📖 Full Article

[STM32 Modbus TCP Server using LWIP – Part 4: Read Coils and Discrete Inputs](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part4-read-coils-discrete-inputs/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32 Nucleo H755ZI |
| Ethernet | Onboard RMII PHY (LAN8742) |
| UART | UART3 — ST-Link virtual COM port |
| ADC Input | 10k potentiometer, wiper on ADC1 channel 15 |
| Discrete Inputs | 5 wires wired to common ground, internal pull-up enabled |
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

> The potentiometer from the earlier parts stays connected exactly as it was. Five new wires are added for the discrete inputs, all tied to a common ground.

---

## What This Project Does

- Continues the Ethernet, LWIP, ADC, and MPU configuration from Part 3 — unchanged in this project
- Adds five GPIO input pins in CubeMX, one for each discrete input, with internal pull-up enabled and GPIO context assigned to Cortex-M7
- Adds a 20-coil database (`MB_Coils`) with default values, and a 5-input discrete input database (`MB_DisInputs`) refreshed from the GPIO pins
- Handles function code 1, reading coil status back to the client at the bit level
- Handles function code 2, reading discrete input status back to the client at the bit level
- Validates every read request against the Modbus PDU limit (max 2000 coils/inputs per request) and the defined database size, returning Illegal Data Value or Illegal Data Address exceptions when a request goes out of range
- Leaves the coil database writable only in code for now — there is no write function code wired up yet, that comes in Part 5

---

## How It Works

Function code 1 pulls the starting address and the requested quantity out of the request data, calculates how many bytes are needed to hold that many coil bits, and validates both the quantity (1 to 2000) and the address range against `MB_COIL_COUNT`. Once validated, it walks through each requested coil one bit at a time, working out which byte and bit inside `MB_Coils` holds that coil's value, and setting the matching bit inside the response buffer. Since the response buffer is cleared before the loop starts, any coil that reads as off is simply left as 0.

Function code 2 follows the exact same logic, except it reads from `MB_DisInputs` instead of `MB_Coils`, and validates the address range against `MB_DIS_INPUT_COUNT` instead. The discrete input database itself is refreshed every 500 milliseconds inside the main loop, by reading the state of the five GPIO pins and packing each one into its corresponding bit using `MB_SetDiscreteInput`.

Both functions reuse the same MBAP header pattern used for the registers in the earlier parts, with the length field adjusted to account for the byte count field instead of a fixed 2-byte-per-register size.

### Key Files

| File | What it does |
|------|--------------|
| `modbus.h` | Function code definitions, exception codes, and the `MB_Request_t` structure — unchanged from Part 1 |
| `Modbus_Database.h` / `.c` | Holding and input register arrays carried over from Part 2/3, plus the new coil and discrete input arrays, `MB_UpdateDiscreteInputs`, and the `MB_SetDiscreteInput` helper |
| `Modbus_Server.c` | Sets up the TCP server — unchanged from Part 1 |
| `Modbus_Parser.c` | Adds `MB_FC01_ReadCoils` and `MB_FC02_ReadDisInputs`, and routes both from `MB_ProcessFunction` |
| `main.c` | Calls `MB_UpdateDiscreteInputs` inside the infinite loop, alongside `MB_UpdateInputRegisters` |

---

## Project Structure

```
modbus/tcp/server/discrete_inputs/
├── CM7/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── main.h
│   │   └── Src/
│   │       └── main.c              ← MB_UpdateDiscreteInputs call added
│   └── Drivers/
│       └── Modbus/
│           ├── modbus.h
│           ├── Modbus_Database.h    ← coil and discrete input counts/arrays added
│           ├── Modbus_Database.c    ← coil/discrete input databases and update logic added
│           ├── Modbus_Server.c
│           ├── Modbus_Server.h
│           ├── Modbus_Parser.c      ← FC01 and FC02 handlers added here
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

**New in this part:** the five discrete input pins (Input1 through Input5). Everything else above is carried over from Part 3 as-is.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. Open the generated project in STM32CubeIDE
4. Add the section relocation for `.RxDescripSection`, `.TxDescripSection`, and `.Rx_PoolSection` to the linker script, placing them in `RAM_D2`
5. Wire a 10k potentiometer — outer pins to 3.3V and GND, wiper to PA3
6. Wire five switches (or jumper wires) from PF14, PF15, PD0, PD1, and PB14 to a common ground
7. Build and flash to the board
8. Set your PC's Ethernet adapter to the same subnet as the board if connecting directly

---

## Testing

Open a serial monitor on the ST-Link virtual COM port at **115200 baud, 8N1**. On reset, the console prints:
```
Initializing Modbus TCP Server...
Port : 502
Server is Listening...
```

Connect using a Modbus TCP client (for example, [Simply Modbus TCP Client](https://simplymodbus.ca/tools-client.html)) using the board's IP address and port 502.

**Function code 1 — Read Coils:** request 20 coils starting from coil 1, with the offset set to 1. This maps to a request for 20 coils starting at address 0. The data returned matches the default values defined in `MB_Coils`. Requesting a smaller range, for example 10 coils starting at coil 10, returns the matching slice of the same database.

**Out-of-range coil read:** request 12 coils starting from coil 10, which reaches into the 21st coil. Since only 20 coils are defined, the server responds with an Illegal Data Address exception:
```
========== Modbus TCP Request ==========
Transaction ID : 1
Protocol ID    : 0
Length         : 6
Unit ID        : 9
Function Code  : 0x01
Sending Exception Response
Exception Code : 0x02
```

**Function code 2 — Read Discrete Inputs:** request 5 inputs starting from address 10001. With all five wires connected to ground, the response comes back as all zeros. Disconnecting a wire flips the matching bit to 1, and removing the common ground connection entirely flips every bit to 1, matching the physical state of the switches in real time.

This confirms both read function codes return the correct bit-level data and that the server rejects any read request that falls outside the defined coil or discrete input range.

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | Ethernet Setup and Basic Server | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part1/) |
| Part 2 | Reading Holding & Input Registers | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part2-registers/) |
| Part 3 | Writing Holding Registers | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part3-write-registers/) |
| **Part 4** | **Reading Coils & Discrete Inputs (this project)** | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part4-read-coils-discrete-inputs/) |
| Part 5 | Writing Coils | Coming soon |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
