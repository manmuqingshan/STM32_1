# STM32 Modbus TCP Client — Basic Setup

Part 6 of the STM32 Modbus TCP series, and the first part on the client side. The previous five parts built a complete Modbus TCP server on the STM32, covering every function code needed to read and write registers, coils, and discrete inputs. Starting with this part, the series switches direction: the STM32 becomes the Modbus TCP client instead of the server. This project only configures Ethernet, brings up LWIP, and connects to a Modbus TCP server to send a single test request — no function code handling yet. That starts in the next part.

## 📺 Video Tutorial

[STM32 Modbus TCP Client using LWIP — Basic Setup | Modbus TCP Series #6](https://youtu.be/leCoD2vb1BU)

## 📖 Full Article

[STM32 Modbus TCP Client using LWIP – Part 6: Setting Up the Client](https://controllerstech.com/stm32-modbus-tcp-client-lwip-part6-client-setup/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32 Nucleo H755ZI |
| Ethernet | Onboard RMII PHY (LAN8742) |
| UART | UART3 — ST-Link virtual COM port |
| Modbus Server | Any Modbus TCP server on port 502 — a PC running a Python script, a Modbus simulator, or the STM32 server built in Parts 1–5 |
| Network | Direct Ethernet connection or router (static IP) |

### Pin Assignment

| Signal | Pin | Function |
|--------|-----|----------|
| Ethernet RMII | Auto-assigned | Configured via CubeMX Connectivity → Ethernet, mode RMII |
| UART3 TX | PD8 | ST-Link virtual COM port TX |
| UART3 RX | PD9 | ST-Link virtual COM port RX |

> There is no additional wiring for this part — no potentiometer, switches, or LEDs. This project is purely a TCP client connecting to a server, with no Modbus data tied to any GPIO yet.

---

## What This Project Does

- Configures Ethernet in CubeMX (RMII mode, Cortex-M7 core only) and brings up the LWIP stack
- Sets up the memory layout for the RX/TX DMA descriptors, RX buffer pool, and the LWIP heap in D2 RAM
- Implements `Modbus_Client_Process`, which waits for the Ethernet link to come up, connects to the configured server IP and port, and retries on a timeout up to a fixed limit
- Registers the connected, receive, and error callbacks once the connection succeeds
- Sends a single test Modbus request (function code 3, reading holding registers) purely to confirm the server receives and decodes it correctly
- Reports the number of bytes received back from the server, without decoding the response yet — actual decoding starts once function codes are added in the next part

---

## How It Works

`Modbus_Client_Process` runs on every pass of the main loop. It first checks that the Ethernet link is up, since the client is the one initiating the connection here, and there is no point calling `tcp_connect` before the link is ready. Once the link is up, `Modbus_Client_Init` creates a TCP control block and calls `tcp_connect` toward the server. If the connection does not succeed within the timeout, the attempt is retried, up to a fixed number of times, after which the client stops trying until the board is reset.

Once connected, the connected callback registers the receive and error callbacks, and immediately sends the test Modbus request. From there, whenever the server sends data back, the receive callback copies it into a local buffer and hands it to `MB_Client_Parser`, which for this part only reports how many bytes came in.

### Key Files

| File | What it does |
|------|--------------|
| `modbus.h` | Function code definitions and exception codes — shared with the server side of this series |
| `Modbus_Client.h` / `Modbus_Client.c` | Sets up the TCP client: creates the control block, connects to the server, handles retries, and registers the connected, receive, and error callbacks |
| `Modbus_Client_Parser.h` / `Modbus_Client_Parser.c` | Parses whatever bytes come back from the server — for this part, only reports the byte count |
| `main.c` | Calls `Modbus_Client_Process` inside the infinite loop, alongside `MX_LWIP_Process` |

---

## Project Structure

```
modbus/tcp/client/client_setup/
├── CM7/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── main.h
│   │   └── Src/
│   │       └── main.c              ← Modbus_Client_Process call added
│   └── Drivers/
│       └── Modbus/
│           ├── modbus.h
│           ├── Modbus_Client.h
│           ├── Modbus_Client.c
│           ├── Modbus_Client_Parser.h
│           └── Modbus_Client_Parser.c
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

**This is a new project** — Ethernet, LWIP, and MPU settings are configured from scratch here, following the same memory layout and PHY setup used throughout the server side of this series.

---

## How to Build

1. Clone this repository
2. Open the `.ioc` file in STM32CubeMX and generate code
3. Open the generated project in STM32CubeIDE
4. Add the section relocation for `.RxDescripSection`, `.TxDescripSection`, and `.Rx_PoolSection` to the linker script, placing them in `RAM_D2`
5. Inside `main.c`, update the IP address and port passed to `Modbus_Client_Process` to match the Modbus TCP server you want to connect to
6. Build and flash to the board
7. Set your PC's Ethernet adapter to the same subnet as the board if connecting directly
8. Run a Modbus TCP server on your PC — a Python script, a Modbus simulator, or the STM32 server project from Parts 1–5 — listening on port 502

---

## Testing

Open a serial monitor on the ST-Link virtual COM port at **115200 baud, 8N1**. If the server is not running yet, the console prints:
```
Starting Modbus Client...
Connecting to the Server...
IP : 192.168.1.10
Port   : 502
Connection Timeout
```

This repeats every five seconds. After five failed attempts, the console prints:
```
Maximum retries reached
```
At this point, the client stops trying to connect, and the board needs to be reset to start over.

**Successful connection:** start the Modbus TCP server first, then reset the board. The console now prints:
```
Connected to Modbus Server
Received x bytes
```
On the server side, confirm it prints the client's IP address and the assigned port, and decodes the incoming request as function code 3, requesting holding registers, exactly as sent from the client code.

**Changing the test request:** edit the function code or requested quantity inside `tcp_client_connected` in `Modbus_Client.c`, rebuild, and flash. The server should print the updated function code and quantity, confirming that whatever the client sends is received and decoded correctly on the other end.

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | Ethernet Setup and Basic Server | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part1/) |
| Part 2 | Reading Holding & Input Registers | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part2-registers/) |
| Part 3 | Writing Holding Registers | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part3-write-registers/) |
| Part 4 | Reading Coils & Discrete Inputs | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part4-read-coils-discrete-inputs/) |
| Part 5 | Writing Coils | [Article](https://controllerstech.com/stm32-modbus-tcp-server-lwip-part5-write-coils/) |
| **Part 6** | **Modbus TCP Client — Basic Setup (this project)** | [Article](https://controllerstech.com/stm32-modbus-tcp-client-lwip-part6-client-setup/) |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
