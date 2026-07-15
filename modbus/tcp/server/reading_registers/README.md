# STM32 Modbus TCP Server — Holding & Input Registers

Part 2 of the STM32 Modbus TCP series. This project builds on the Part 1 server and adds function code 3 (Read Holding Registers) and function code 4 (Read Input Registers). Holding registers are backed by a fixed sample database, while input registers are fed from a live ADC reading taken from a potentiometer connected to the board. Writing registers and coils is added in a later part of this series.

## 📺 Video Tutorial

[STM32 Modbus TCP Server using LWIP — Holding & Input Registers | Modbus TCP Series #2](https://youtu.be/P2xmSv1posk)

## 📖 Full Article

[STM32 Modbus TCP Server using LWIP – Part 2: Holding & Input Registers](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part2-registers/)

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

> Always compare the Ethernet pins CubeMX assigns against your board's schematic. CubeMX sometimes selects the wrong pins for the RMII interface.

---

## What This Project Does

- Continues the Ethernet, LWIP, and MPU configuration from Part 1 — unchanged in this project
- Adds ADC1 in CubeMX, channel 15, 16-bit resolution, blocking mode, clocked from the peripheral clock at 64 MHz
- Introduces a Modbus register database (`Modbus_Database.c` / `Modbus_Database.h`) — 10 holding registers and 10 input registers
- Handles function code 3, returning fixed sample values from the holding register array
- Handles function code 4, returning live values from the input register array
- Updates the input register array every 500 ms from the main loop — uptime, raw ADC value, and inverted ADC value
- Validates every request against the Modbus PDU limit (max 125 registers) and the defined register count, returning Illegal Data Value or Illegal Data Address exceptions when a request goes out of range

---

## How It Works

Function code 3 and function code 4 both read from a register array, the only difference is which array they read from and whether that array holds fixed data or live hardware data. Holding registers are set once and stay the same until the code changes them. Input registers act as a buffer between the Modbus layer and a piece of hardware, and are meant to be refreshed on their own.

`MB_UpdateInputRegisters` handles this refresh. It calls `readADC`, defined in `main.c`, to read the potentiometer through ADC1 channel 15, then stores the uptime, the raw ADC value, and the inverted ADC value into the input register array. This function runs from the main loop every 500 ms, using a tick check instead of `HAL_Delay`, so `MX_LWIP_Process` never gets held up.

When a request for function code 3 or function code 4 comes in, the parser pulls the starting address and quantity out of the request, checks that the quantity is between 1 and 125 and that the requested range fits inside the register count, then copies the matching register values into the response buffer before sending it back over TCP.

### Key Files

| File | What it does |
|------|--------------|
| `modbus.h` | Function code definitions, exception codes, and the `MB_Request_t` structure — unchanged from Part 1 |
| `Modbus_Database.h` | Defines `MB_HOLDING_REG_COUNT` and `MB_INPUT_REG_COUNT` |
| `Modbus_Database.c` | Holds the holding register sample values, the input register array, and `MB_UpdateInputRegisters` |
| `Modbus_Server.c` | Sets up the TCP server — unchanged from Part 1 |
| `Modbus_Parser.c` | Adds `MB_FC03_ReadHoldingRegisters` and `MB_FC04_ReadInputRegisters`, and routes both from `MB_ProcessFunction` |

---

## Project Structure

```
modbus/tcp/server/reading_registers/
├── CM7/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── main.h
│   │   └── Src/
│   │       └── main.c              ← readADC(), prevTick update, Modbus_Server_Init(), MX_LWIP_Process()
│   └── Drivers/
│       └── Modbus/
│           ├── modbus.h
│           ├── Modbus_Database.h
│           ├── Modbus_Database.c
│           ├── Modbus_Server.c
│           ├── Modbus_Server.h
│           ├── Modbus_Parser.c
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

**Memory layout note:** RX DMA descriptors start at `0x30000000`, TX descriptors at `0x30000080`, and RX buffers at `0x30000100`, spanning roughly 18 KB. The LWIP heap should start right after this, at `0x30004900` — not the CubeMX default — to avoid overlapping with the RX buffer. This is unchanged from Part 1.

**ADC clock note:** Enabling ADC1 channel 15 at 16-bit resolution can push the ADC clock past its allowed frequency, showing an error in the Clock Configuration tab. Switching the ADC clock source to the peripheral clock brings it down to 64 MHz and clears the error.

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

**Function code 3 — Read Holding Registers:** set the first register to `40001` and the quantity to `10`. The response should match the sample values stored in `MB_HoldingRegs`.

**Function code 4 — Read Input Registers:** set the first register to `30001` and the quantity to `10`. The first three registers return the uptime in seconds, the raw ADC value, and the inverted ADC value. Rotate the potentiometer and send the request again — the ADC-based registers should update.

**Out-of-range request:** ask for more registers than the database has defined (for example, quantity `7` starting from `40005`), and the server responds with an Illegal Data Address exception:
```
========== Modbus TCP Request ==========
Transaction ID : 1
Protocol ID    : 0
Length         : 6
Unit ID        : 9
Function Code  : 0x03
Sending Exception Response
Exception Code : 0x02
```

This confirms both function codes return the correct data and that the server rejects any request that falls outside the defined register range.

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | Ethernet Setup and Basic Server | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part1/) |
| **Part 2** | **Reading Holding & Input Registers (this project)** | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part2-registers/) |
| Part 3 | Writing Registers & Coils | Coming soon |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
