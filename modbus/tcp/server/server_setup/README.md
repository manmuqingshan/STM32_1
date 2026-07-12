# STM32 Modbus TCP Server — Ethernet Setup and Basic Server

Part 1 of the STM32 Modbus TCP series. This project configures an STM32 Nucleo H755 as a Modbus TCP server using the LWIP stack over Ethernet. It sets up the Ethernet peripheral, static IP, and MPU region for cache coherency, then implements a basic Modbus TCP server that parses incoming requests and responds with a Modbus exception. Function code handling (reading and writing registers/coils) is added in later parts of this series.

## 📺 Video Tutorial

[STM32 Modbus TCP Server using LWIP — Ethernet Setup and Basic Server | Modbus TCP Series #1](https://youtu.be/uQCWLD-066s)

## 📖 Full Article

[STM32 Modbus TCP Server using LWIP – Part 1: Ethernet Setup and Basic Server](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part1/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32 Nucleo H755ZI |
| Ethernet | Onboard RMII PHY (LAN8742) |
| UART | UART3 — ST-Link virtual COM port |
| Network | Direct Ethernet connection or router (static IP) |

### Pin Assignment

| Signal | Pin | Function |
|--------|-----|----------|
| Ethernet RMII | Auto-assigned | Configured via CubeMX Connectivity → Ethernet, mode RMII |
| UART3 TX | PD8 | ST-Link virtual COM port TX |
| UART3 RX | PD9 | ST-Link virtual COM port RX |

> Always compare the Ethernet pins CubeMX assigns against your board's schematic. CubeMX sometimes selects the wrong pins for the RMII interface.

---

## What This Project Does

- Configures the Ethernet peripheral in RMII mode using STM32CubeMX
- Enables LWIP for the Cortex-M7 core with a static IP address
- Places Ethernet DMA descriptors and RX buffers in D2 RAM through the linker script
- Configures the MPU to mark this region non-cacheable, avoiding CPU/DMA cache coherency issues
- Implements a Modbus TCP server (`Modbus_Server.c`) that listens on port 502
- Parses incoming requests — MBAP header and PDU — using a dedicated parser (`Modbus_Parser.c`)
- Responds to every request with an Illegal Function exception, since no function code is handled yet

---

## How It Works

A client connects to the server on port 502. The accept callback fires first and registers the receive and error callbacks on that connection. From there, every time the client sends data, the receive callback runs — it acknowledges the received bytes, copies them out of the internal buffer into a local array, and hands that array to the parser.

The parser reads the MBAP header (transaction ID, protocol ID, length, unit ID) and the function code, storing all of it in a single request structure. Since no function code is implemented yet, the request always falls through to the same path, and an exception response is built and sent back — this proves the server is decoding requests correctly before any real logic is added on top.

### Key Files

| File | What it does |
|------|--------------|
| `modbus.h` | Function code definitions, exception codes, and the `MB_Request_t` structure |
| `Modbus_Server.c` | Sets up the TCP server — creates the PCB, binds port 502, registers accept/receive/error callbacks |
| `Modbus_Parser.c` | Parses the MBAP header and function code, builds and sends the exception response |

---

## Project Structure

```
modbus/tcp/server/server_setup/
├── CM7/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── main.h
│   │   └── Src/
│   │       └── main.c              ← _write() for printf, Modbus_Server_Init(), MX_LWIP_Process()
│   └── Drivers/
│       └── Modbus/
│           ├── modbus.h
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

**Memory layout note:** RX DMA descriptors start at `0x30000000`, TX descriptors at `0x30000080`, and RX buffers at `0x30000100`, spanning roughly 18 KB. The LWIP heap should start right after this, at `0x30004900` — not the CubeMX default — to avoid overlapping with the RX buffer.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. Open the generated project in STM32CubeIDE
4. Add the section relocation for `.RxDescripSection`, `.TxDescripSection`, and `.Rx_PoolSection` to the linker script, placing them in `RAM_D2`
5. Build and flash to the board
6. Set your PC's Ethernet adapter to the same subnet as the board if connecting directly

---

## Testing

Open a serial monitor on the ST-Link virtual COM port at **115200 baud, 8N1**. On reset, the console prints:
```
Initializing Modbus TCP Server...
Port : 502
Server is Listening...
```

Connect using a Modbus TCP client (for example, [Simply Modbus TCP Client](https://simplymodbus.ca/tools-client.html)) using the board's IP address and port 502. Send any function code request — the console logs the decoded request, and the client receives an Illegal Function exception in response:
```
========== Modbus TCP Request ==========
Transaction ID : 1
Protocol ID    : 0
Length         : 6
Unit ID        : 9
Function Code  : 0x03
Sending Exception Response
Exception Code : 0x01
```

This confirms the server correctly decodes requests and responds, even before any function code logic is added.

---

## Series

| Part | Topic | Link |
|------|-------|------|
| **Part 1** | **Ethernet Setup and Basic Server (this project)** | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part1/) |
| Part 2 | Reading Holding & Input Registers | Coming soon |
| Part 3 | Writing Registers & Coils | Coming soon |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
