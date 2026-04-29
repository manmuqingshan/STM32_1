# STM32 Modbus TCP Server – Read and Write Coils

This project demonstrates how to configure the **STM32 Nucleo H755ZI** as a **Modbus TCP server** using the Mongoose networking library. The server handles FC01 (read coils), FC05 (write single coil), and FC15 (write multiple coils) requests from a Modbus TCP client. It also exposes a **Mongoose web UI dashboard** where coil states can be read and toggled in real time via WebSocket.

This is **Part 2** of the STM32 Modbus TCP series.

---

## Full Tutorial and Explanation

Step-by-step explanation, CubeMX screenshots, and video walkthrough available at:

- https://controllerstech.com/stm32-modbus-tcp-server-coils/
- https://youtu.be/4aQqukiGj9w

---

## Features Covered

- Updating the Mongoose web UI wizard project from Part 1 (inputs → coils)
- Reconfiguring 10 GPIO pins from input to output in STM32CubeMX
- Implementing a bit-packed `Coils_Data` byte array as a shared coil database
- Writing `READ_COIL()` and `WRITE_COIL()` functions using bit shift and mask operations
- Writing a custom Modbus handler for FC01, FC05, and FC15
- Reading and writing coils from both the Modbus TCP client and the Mongoose web UI
- Registering custom getter and setter functions for the web UI
- Testing partial reads and writes (non-zero starting address)
- Analyzing raw Modbus TCP request and response frames byte by byte

---

## How Coils Work

Each GPIO pin drives an LED. The coil state is stored as a single bit in `Coils_Data[]`.

| Coil State | Bit Value | LED State |
|---|---|---|
| Written 1 (true) | 1 | ON |
| Written 0 (false) | 0 | OFF |

Coils can be modified from two sources — the Modbus TCP client and the Mongoose web UI. Both sources go through `READ_COIL()` and `WRITE_COIL()`, so the state is always consistent.

---

## Tested On

| Board | Core Used | Interface |
|---|---|---|
| STM32 Nucleo H755ZI | Cortex-M7 (CM7) | Ethernet (RMII) |

---

## IP Address Configuration

| Device | IP Address | Port |
|---|---|---|
| STM32 (Server) | `192.168.0.10` | `502` |
| PC (Client) | `192.168.0.2` | — |

Static IP is configured in `mongoose_config.h`:

```c
#define MG_TCPIP_IP   MG_IPV4(192, 168, 0, 10)
#define MG_TCPIP_GW   MG_IPV4(192, 168, 0, 1)
#define MG_TCPIP_MASK MG_IPV4(255, 255, 255, 0)
```

---

## How It Works

```
1. Mongoose wizard project updated from Part 1 — endpoint renamed from inputs to coils
2. STM32CubeMX reconfigures 10 GPIO pins as outputs, renamed COIL1–COIL10
3. Coils_Data[2] stores all 10 coil states as bits (1 bit per coil, 2 bytes total)
4. coil_t array maps each coil index to its GPIO port and pin
5. READ_COIL(i) extracts the correct bit from Coils_Data using byte and bit position
6. WRITE_COIL(i, val) updates the correct bit in Coils_Data and drives the GPIO pin
7. my_modbus_handler() handles FC01, FC05, and FC15 from the Modbus TCP client
8. my_get_coils() syncs my_coils[] from Coils_Data before sending to the web UI
9. my_set_coils() writes incoming web UI changes back to Coils_Data via WRITE_COIL()
10. mongoose_add_ws_reporter() pushes coil state to the web UI every 200ms
```

---

## Supported Function Codes

| Function Code | Operation | Description |
|---|---|---|
| FC01 | Read Coils | Read one or more coil states |
| FC05 | Write Single Coil | Write one coil at a given address |
| FC15 | Write Multiple Coils | Write a range of coils starting from a given address |

---

## Key Code

### Coil Database and Structure

```c
uint8_t Coils_Data[2] = {0x00, 0x00};

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} coil_t;

coil_t coils[] = {
    {COIL1_GPIO_Port,  COIL1_Pin},
    {COIL2_GPIO_Port,  COIL2_Pin},
    {COIL3_GPIO_Port,  COIL3_Pin},
    {COIL4_GPIO_Port,  COIL4_Pin},
    {COIL5_GPIO_Port,  COIL5_Pin},
    {COIL6_GPIO_Port,  COIL6_Pin},
    {COIL7_GPIO_Port,  COIL7_Pin},
    {COIL8_GPIO_Port,  COIL8_Pin},
    {COIL9_GPIO_Port,  COIL9_Pin},
    {COIL10_GPIO_Port, COIL10_Pin},
};
```

### READ_COIL and WRITE_COIL

```c
bool READ_COIL(uint16_t i)
{
    uint16_t startByte   = i / 8;
    uint8_t  bitPosition = i % 8;
    return (Coils_Data[startByte] >> bitPosition) & 0x01;
}

void WRITE_COIL(uint16_t i, bool val)
{
    uint16_t startByte   = i / 8;
    uint8_t  bitPosition = i % 8;

    if (val == true)  Coils_Data[startByte] |=  (1 << bitPosition);
    else              Coils_Data[startByte] &= ~(1 << bitPosition);

    HAL_GPIO_WritePin(coils[i].port, coils[i].pin, val);
}
```

### Modbus Handler (FC01, FC05, FC15)

```c
void my_modbus_handler(struct mg_modbus_req *req)
{
    if (req->func == MG_MODBUS_FUNC_READ_COILS) {
        for (uint16_t i = 0; i < req->len; i++) {
            req->u.bits[i] = READ_COIL(req->addr + i);
        }
    }
    else if (req->func == MG_MODBUS_FUNC_WRITE_SINGLE_COIL) {
        WRITE_COIL(req->addr, req->u.bits[0]);
    }
    else if (req->func == MG_MODBUS_FUNC_WRITE_MULTIPLE_COILS) {
        for (uint16_t i = 0; i < req->len; i++) {
            WRITE_COIL(req->addr + i, req->u.bits[i]);
        }
    }
    else {
        req->error = MG_MODBUS_ERR_DEVICE_FAILURE;
    }
}
```

### Web UI Getter and Setter

```c
static struct coils my_coils[] = {
    {false},{false},{false},{false},{false},
    {false},{false},{false},{false},{false},
};

bool my_get_coils(struct coils *data, size_t i)
{
    size_t array_size = sizeof(my_coils) / sizeof(my_coils[0]);
    if (i >= array_size) return false;

    for (int j = 0; j < array_size; j++) {
        my_coils[j].level = READ_COIL(j);
    }

    *data = my_coils[i];
    return true;
}

void my_set_coils(struct coils *data, size_t i)
{
    size_t array_size = sizeof(my_coils) / sizeof(my_coils[0]);
    if (i < array_size) my_coils[i] = *data;

    for (int j = 0; j < array_size; j++) {
        WRITE_COIL(j, my_coils[j].level);
    }
}
```

### Registration in main()

```c
mongoose_init();
mongoose_set_modbus_handler(my_modbus_handler);
mongoose_set_http_handlers("coils", my_get_coils, my_set_coils);
mongoose_add_ws_reporter(200, "coils");
```

---

## Modbus TCP Frame – FC01 (Read Coils)

### Request (Client → Server)

| Field | Size | Value |
|---|---|---|
| Transaction ID | 2 bytes | 0x0001 |
| Protocol ID | 2 bytes | 0x0000 |
| Length | 2 bytes | 0x0006 |
| Unit ID | 1 byte | 0x0A |
| Function Code | 1 byte | 0x01 |
| Start Address | 2 bytes | 0x0000 |
| Quantity | 2 bytes | 0x000A |

### Response (Server → Client)

| Field | Size | Value |
|---|---|---|
| Transaction ID | 2 bytes | 0x0001 |
| Protocol ID | 2 bytes | 0x0000 |
| Length | 2 bytes | 0x0005 |
| Unit ID | 1 byte | 0x0A |
| Function Code | 1 byte | 0x01 |
| Byte Count | 1 byte | 0x02 |
| Coil Data | 2 bytes | states |

---

## Modbus Client Configuration (Simply Modbus TCP)

| Setting | Value |
|---|---|
| Mode | TCP |
| Server IP | 192.168.0.10 |
| Port | 502 |
| Slave ID | 10 |
| Function Code | 1 (Read Coils) / 5 (Write Single) / 15 (Write Multiple) |
| Starting Address | 1 |
| Quantity | 10 |

---

## Common Errors and Fixes

| Symptom | Likely Cause | Fix |
|---|---|---|
| LEDs do not respond | Pins still set as input | Re-check CubeMX GPIO mode — must be Output |
| Client reads wrong values | Address offset mismatch | Modbus uses 0-based PDU addressing — start address 0 = coil index 0 |
| Web UI toggle has no effect | Setter not registered | Ensure `my_set_coils` is passed to `mongoose_set_http_handlers()` |
| Modbus client and web UI out of sync | Getter not reading from Coils_Data | Ensure `my_get_coils()` calls `READ_COIL()` in the sync loop |
| FC05 write has no effect | Wrong handler branch | Check the `MG_MODBUS_FUNC_WRITE_SINGLE_COIL` condition is present |
| Build errors after wizard regeneration | Modified Mongoose folder | Never edit files inside the Mongoose folder directly |

---

## Related Tutorials

| Part | Topic | Link |
|---|---|---|
| Prerequisite | Modbus TCP Protocol Explained | https://controllerstech.com/modbus-tcp-protocol-explained/ |
| Part 1 | Modbus TCP Server – Read Discrete Inputs | https://controllerstech.com/stm32-modbus-tcp-server-read-discrete-inputs/ |
| Part 2 | Modbus TCP Server – Read/Write Coils | https://controllerstech.com/stm32-modbus-tcp-server-coils/ |
| Part 3 | Modbus TCP Server – Holding Registers | Coming Soon |

---

## License

This example is provided for educational purposes under Controllerstech Guidelines.
