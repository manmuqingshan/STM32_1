# STM32 Modbus TCP Server – Read and Write Holding Registers

This project demonstrates how to configure the **STM32 Nucleo H755ZI** as a **Modbus TCP server** using the Mongoose networking library. The server handles FC03 (read holding registers), FC06 (write single register), and FC16 (write multiple registers) requests from a Modbus TCP client. It also exposes a **Mongoose web UI dashboard** where register values can be viewed and updated in real time via WebSocket.

This is **Part 3** of the STM32 Modbus TCP series.

---

## Full Tutorial and Explanation

Step-by-step explanation, CubeMX screenshots, and video walkthrough available at:

- https://controllerstech.com/stm32-modbus-tcp-server-holding-registers/
- https://youtu.be/rfgx_StZ3ls

---

## Features Covered

- Updating the Mongoose web UI wizard project from Part 2 (coils → holding registers)
- Implementing a `uint16_t` array as a shared holding register database
- Writing `READ_Holding_REG()` and `WRITE_Holding_REG()` functions
- Writing a custom Modbus handler for FC03, FC06, and FC16
- Reading and writing registers from both the Modbus TCP client and the Mongoose web UI
- Registering custom getter and setter functions for the web UI
- Analyzing raw Modbus TCP request and response frames byte by byte

---

## How Holding Registers Work

Each register stores a 16-bit unsigned integer value. Unlike coils (1-bit), no bit packing is needed — each register maps directly to one array element.

| Operation | Source | Function Used |
|---|---|---|
| Read register | Modbus client / Web UI | `READ_Holding_REG()` |
| Write register | Modbus client / Web UI | `WRITE_Holding_REG()` |

Registers can be modified from two sources — the Modbus TCP client and the Mongoose web UI. Both go through `READ_Holding_REG()` and `WRITE_Holding_REG()`, so the state is always consistent.

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
1. Mongoose wizard project updated from Part 2 — endpoint renamed from coils to holding_registers
2. Holding_Reg_Data[12] stores all 12 register values as uint16_t elements
3. READ_Holding_REG(i) returns the value at index i directly from the array
4. WRITE_Holding_REG(i, val) updates the value at index i in the array
5. my_modbus_handler() handles FC03, FC06, and FC16 from the Modbus TCP client
6. my_get_holding_registers() syncs my_holding_registers[] from the database before sending to the web UI
7. my_set_holding_registers() writes incoming web UI changes back to the database via WRITE_Holding_REG()
8. mongoose_add_ws_reporter() pushes register values to the web UI every 200ms
```

---

## Supported Function Codes

| Function Code | Operation | Description |
|---|---|---|
| FC03 | Read Holding Registers | Read one or more register values |
| FC06 | Write Single Register | Write one register at a given address |
| FC16 | Write Multiple Registers | Write a range of registers starting from a given address |

---

## Key Code

### Register Database

```c
uint16_t Holding_Reg_Data[12] = {1111, 2222, 3333, 4444, 5555, 6666,
                                   7777, 8888, 9999, 1234, 5678, 9012};
```

### READ and WRITE Functions

```c
uint16_t READ_Holding_REG(uint16_t i)
{
    return (Holding_Reg_Data[i]);
}

void WRITE_Holding_REG(uint16_t i, uint16_t val)
{
    Holding_Reg_Data[i] = val;
}
```

### Modbus Handler (FC03, FC06, FC16)

```c
void my_modbus_handler(struct mg_modbus_req *req)
{
    if (req->func == MG_MODBUS_FUNC_READ_HOLDING_REGISTERS) {
        for (uint16_t i = 0; i < req->len; i++) {
            req->u.regs[i] = READ_Holding_REG(req->addr + i);
        }
    }
    else if (req->func == MG_MODBUS_FUNC_WRITE_MULTIPLE_REGISTERS) {
        for (uint16_t i = 0; i < req->len; i++) {
            WRITE_Holding_REG(req->addr + i, req->u.regs[i]);
        }
    }
    else if (req->func == MG_MODBUS_FUNC_WRITE_SINGLE_REGISTER) {
        WRITE_Holding_REG(req->addr, req->u.regs[0]);
    }
    else {
        req->error = MG_MODBUS_ERR_DEVICE_FAILURE;
    }
}
```

### Web UI Getter and Setter

```c
static struct holding_registers my_holding_registers[] = {
    {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},
};

bool my_get_holding_registers(struct holding_registers *data, size_t i)
{
    size_t array_size = sizeof(my_holding_registers) / sizeof(my_holding_registers[0]);
    if (i >= array_size) return false;
    my_holding_registers[i].value = READ_Holding_REG(i);
    *data = my_holding_registers[i];
    return true;
}

void my_set_holding_registers(struct holding_registers *data, size_t i)
{
    size_t array_size = sizeof(my_holding_registers) / sizeof(my_holding_registers[0]);
    if (i < array_size) my_holding_registers[i] = *data;
    WRITE_Holding_REG(i, my_holding_registers[i].value);
}
```

### Registration in main()

```c
mongoose_init();
mongoose_set_modbus_handler(my_modbus_handler);
mongoose_set_http_handlers("holding_registers", my_get_holding_registers, my_set_holding_registers);
mongoose_add_ws_reporter(200, "holding_registers");
for (;;) {
    mongoose_poll();
}
```

---

## Modbus TCP Frame – FC03 (Read Holding Registers)

### Request (Client → Server)

| Field | Size | Value | Description |
|---|---|---|---|
| Transaction ID | 2 bytes | 0x0001 | First transaction |
| Protocol ID | 2 bytes | 0x0000 | Always 0 for Modbus |
| Length | 2 bytes | 0x0006 | 6 bytes follow |
| Unit ID | 1 byte | 0x0A | Slave ID |
| Function Code | 1 byte | 0x03 | Read holding registers |
| Start Address | 2 bytes | 0x0000 | Register 40001 |
| Quantity | 2 bytes | 0x000C | 12 registers |

### Response (Server → Client)

| Field | Size | Value | Description |
|---|---|---|---|
| Transaction ID | 2 bytes | 0x0001 | Matches request |
| Protocol ID | 2 bytes | 0x0000 | Always 0 for Modbus |
| Length | 2 bytes | 0x001B | 27 bytes follow |
| Unit ID | 1 byte | 0x0A | Slave ID |
| Function Code | 1 byte | 0x03 | Read holding registers |
| Byte Count | 1 byte | 0x18 | 24 data bytes |
| Register Data | 24 bytes | — | 12 registers × 2 bytes each |

---

## Modbus Client Configuration (Simply Modbus TCP)

| Setting | Value |
|---|---|
| Mode | TCP |
| Server IP | 192.168.0.10 |
| Port | 502 |
| Slave ID | 10 |
| Function Code | 3 (Read) / 6 (Write Single) / 16 (Write Multiple) |
| Starting Address | 40001 |
| Quantity | 12 |
| Data Type | 16-bit unsigned |

---

## Common Errors and Fixes

| Symptom | Likely Cause | Fix |
|---|---|---|
| Client reads all zeros | Database not initialized | Set initial values in `Holding_Reg_Data[]` |
| Web UI shows stale values | Getter not syncing from database | Ensure `READ_Holding_REG(i)` is called inside `my_get_holding_registers()` |
| FC06 write has no effect | Wrong handler branch | Check `MG_MODBUS_FUNC_WRITE_SINGLE_REGISTER` condition is present |
| FC16 writes only first register | Loop missing | Ensure the `for` loop is used for `WRITE_MULTIPLE_REGISTERS` |
| Modbus client and web UI out of sync | Setter not writing to database | Ensure `my_set_holding_registers()` calls `WRITE_Holding_REG()` |
| Build errors after wizard regeneration | Modified Mongoose folder | Never edit files inside the Mongoose folder directly |

---

## Related Tutorials

| Part | Topic | Link |
|---|---|---|
| Prerequisite | Modbus TCP Protocol Explained | https://controllerstech.com/modbus-tcp-protocol-explained/ |
| Part 1 | Modbus TCP Server – Read Discrete Inputs | https://controllerstech.com/stm32-modbus-tcp-server-read-discrete-inputs/ |
| Part 2 | Modbus TCP Server – Read/Write Coils | https://controllerstech.com/stm32-modbus-tcp-server-coils/ |
| Part 3 | Modbus TCP Server – Holding Registers | https://controllerstech.com/stm32-modbus-tcp-server-holding-registers/ |
| Part 4 | Modbus TCP Server – Input Registers | Coming Soon |

---

## License

This example is provided for educational purposes under Controllerstech Guidelines.
