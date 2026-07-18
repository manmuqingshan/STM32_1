# STM32 Modbus TCP Server — Writing Holding Registers

Part 3 of the STM32 Modbus TCP series. This project builds on the Part 2 server and adds function code 6 (Write Single Register) and function code 16 (Write Multiple Registers). Both function codes write into the same holding register database introduced in Part 2. Input registers remain read-only, since their data is fed from the ADC and not meant to be written by a client.

## 📺 Video Tutorial

[STM32 Modbus TCP Server using LWIP — Writing Holding Registers | Modbus TCP Series #3](https://youtu.be/TTc_aT527Z0)

## 📖 Full Article

[STM32 Modbus TCP Server using LWIP – Part 3: Writing Holding Registers](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part3-write-registers/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32 Nucleo H755ZI |
| Ethernet | Onboard RMII PHY (LAN8742) |
| UART | UART3 — ST-Link virtual COM port |
| ADC Input | 10k potentiometer, wiper on ADC1 channel 15 |
| Network | Direct Ethernet connection or router (static IP) |

### Pin Assignment

| Signal | Pin | Function |
|--------|-----|----------|
| Ethernet RMII | Auto-assigned | Configured via CubeMX Connectivity → Ethernet, mode RMII |
| UART3 TX | PD8 | ST-Link virtual COM port TX |
| UART3 RX | PD9 | ST-Link virtual COM port RX |
| ADC1 Channel 15 | PA3 | Potentiometer wiper input |

> No new hardware or pin assignments are added in this part. The potentiometer from Part 2 stays connected exactly as it was.

---

## What This Project Does

- Continues the Ethernet, LWIP, ADC, and MPU configuration from Part 2 — unchanged in this project
- No CubeMX changes in this part — only `Modbus_Parser.c` is modified
- Handles function code 6, writing a single value into the holding register database and echoing the request back as the response
- Handles function code 16, writing multiple values into the holding register database in one request and responding with the starting address and quantity written
- Validates every write request against the Modbus PDU limit (max 123 registers for FC16) and the defined register count, returning Illegal Data Value or Illegal Data Address exceptions when a request goes out of range
- Leaves the input register database read-only — there is no write handler for `MB_InputRegs`

---

## How It Works

Function code 6 pulls the register address and the value straight out of the request data, checks that the address falls inside `MB_HOLDING_REG_COUNT`, and then writes the value into `MB_HoldingRegs` at that address. The response PDU mirrors the request exactly, address and value unchanged, which is how the client confirms the write went through.

Function code 16 pulls the starting address, quantity, and byte count out of the request. After checking that the quantity is between 1 and 123 and that the requested range fits inside the register count, it loops through the incoming data, combines each pair of bytes into a 16-bit value, and writes it into `MB_HoldingRegs` starting at the given address. The response only confirms the function code, starting address, and quantity written — it does not echo the data back, since the client already has it.

Both functions reuse the same `MB_HoldingRegs` array and the same `MB_HOLDING_REG_COUNT` bound that function code 3 already validates against, so a write is rejected under the same conditions a read would be.

### Key Files

| File | What it does |
|------|--------------|
| `modbus.h` | Function code definitions, exception codes, and the `MB_Request_t` structure — unchanged from Part 1 |
| `Modbus_Database.h` / `.c` | Holding and input register arrays, `MB_UpdateInputRegisters` — unchanged from Part 2 |
| `Modbus_Server.c` | Sets up the TCP server — unchanged from Part 1 |
| `Modbus_Parser.c` | Adds `MB_FC06_WriteSingleRegister` and `MB_FC16_WriteMultipleRegisters`, and routes both from `MB_ProcessFunction` |

---

## Project Structure

```
modbus/tcp/server/writing_registers/
├── CM7/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── main.h
│   │   └── Src/
│   │       └── main.c              ← unchanged from Part 2
│   └── Drivers/
│       └── Modbus/
│           ├── modbus.h
│           ├── Modbus_Database.h
│           ├── Modbus_Database.c
│           ├── Modbus_Server.c
│           ├── Modbus_Server.h
│           ├── Modbus_Parser.c      ← FC06 and FC16 handlers added here
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

**No configuration changes in this part.** Everything above is carried over from Part 2 as-is — there is nothing new to generate from CubeMX.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. Open the generated project in STM32CubeIDE
4. Add the section relocation for `.RxDescripSection`, `.TxDescripSection`, and `.Rx_PoolSection` to the linker script, placing them in `RAM_D2`
5. Wire a 10k potentiometer — outer pins to 3.3V and GND, wiper to PA3
6. Build and flash to the board
7. Set your PC's Ethernet adapter to the same subnet as the board if connecting directly

---

## Testing

Open a serial monitor on the ST-Link virtual COM port at **115200 baud, 8N1**. On reset, the console prints:
```
Initializing Modbus TCP Server...
Port : 502
Server is Listening...
```

Connect using a Modbus TCP client (for example, [Simply Modbus TCP Client](https://simplymodbus.ca/tools-client.html)) using the board's IP address and port 502.

**Function code 6 — Write Single Register:** set the register to `40005` and write a value, for example `12345`. The response returned to the client matches the request exactly. Reading holding registers afterward (function code 3) confirms register 4 now holds the new value.

**Function code 16 — Write Multiple Registers:** set the starting register to `40005` and write 6 registers at once. The console logs the decoded starting address, quantity, and byte count, followed by each value written:
```
FC16 - Write Multiple Holding Registers
Start Address : 4
Quantity      : 6
Byte Count    : 12
Reg[4] = ...
Reg[5] = ...
```
Reading holding registers afterward confirms registers 4 through 9 hold the new values.

**Out-of-range write:** ask to write registers that fall outside the database (for example, 4 registers starting from `40008`, which reaches register `40011`), and the server responds with an Illegal Data Address exception:
```
========== Modbus TCP Request ==========
Transaction ID : 1
Protocol ID    : 0
Length         : 15
Unit ID        : 9
Function Code  : 0x10
Sending Exception Response
Exception Code : 0x02
```

**Input registers stay read-only:** reading input registers with function code 4 still works as expected, confirming there is no write path into `MB_InputRegs`.

This confirms both write function codes update the database correctly and that the server rejects any write request that falls outside the defined register range.

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | Ethernet Setup and Basic Server | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part1/) |
| Part 2 | Reading Holding & Input Registers | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part2-registers/) |
| **Part 3** | **Writing Holding Registers (this project)** | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part3-write-registers/) |
| Part 4 | Coils & Discrete Inputs | Coming soon |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
