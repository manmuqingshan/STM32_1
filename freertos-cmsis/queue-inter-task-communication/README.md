# STM32 FreeRTOS Queue — Inter-Task Communication

This project demonstrates how to use FreeRTOS message queues on STM32 for safe inter-task communication using CMSIS-OS v2. Two sender tasks push a struct into a queue, and a receiver task reads from it and prints the output via UART using `printf`.

---

## 📺 Tutorial

**[Watch the full video tutorial on YouTube →](https://youtu.be/wMp5GQLobio)**

**[Read the full written guide →](https://controllerstech.com/stm32-freertos-queue-inter-task-communication/)**

---

## 📋 Overview

A common mistake in FreeRTOS applications is using global variables to share data between tasks. This leads to race conditions and corrupted data. This project shows the correct approach — using a message queue to pass a struct between tasks safely.

- **StartTask1** — Monitors button on PA3 and pushes a struct into the queue on each press
- **StartTask2** — Automatically pushes a struct into the queue every 1 second
- **StartRxTask** — Reads from the queue and prints each message via UART using `printf`

![Serial console output showing Event ID 2 messages every second from Task 2 and Event ID 1 messages with millisecond timestamps from Task 1 when the button is pressed](images/queue_result.webp)

---

## 🛠 Hardware

- STM32L496 Nucleo board (any STM32 Nucleo board works with minor pin changes)
- Push button connected between **PA3** and **GND**
- USB cable for Virtual COM port (ST-LINK)

---

## ⚙️ CubeMX Configuration

| Parameter | Value |
|-----------|-------|
| RTOS | FreeRTOS (CMSIS-OS v2) |
| Heap Size | 5 KB |
| Queue Handle | `messageQueueHandle` |
| Queue Size | 10 elements |
| Queue Item Size | `sizeof(messageQueue_t)` |
| UART | LPUART1 — PG7 (TX), PG8 (RX) |
| Baud Rate | 115200, 8N1 |
| Button Pin | PA3 — GPIO Input, Pull-Up |

### Task Configuration

| Task | Priority | Stack Size | Role |
|------|----------|------------|------|
| StartTask1 | Normal | 128 words | Sends struct on button press |
| StartTask2 | Normal | 128 words | Sends struct every 1 second |
| StartRxTask | High | 512 words | Reads queue, prints via UART |

---

## 📁 Message Structure

```c
typedef struct {
    uint8_t  event_id;    // 0x01 = Task1, 0x02 = Task2
    uint32_t timestamp;   // HAL_GetTick() — ms for Task1, seconds for Task2
} messageQueue_t;
```

---

## 💻 Task Code

### Task 1 — Send on Button Press

```c
void StartTask1(void *argument)
{
    messageQueue_t msg;

    for (;;)
    {
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_3) == 0)
        {
            msg.event_id  = 0x01;
            msg.timestamp = HAL_GetTick();

            osMessageQueuePut(messageQueueHandle, &msg, 0, 0);

            osDelay(200);  // debounce
        }
        osDelay(20);
    }
}
```

### Task 2 — Send Every Second

```c
void StartTask2(void *argument)
{
    messageQueue_t msg;

    for (;;)
    {
        msg.event_id  = 0x02;
        msg.timestamp = HAL_GetTick() / 1000;  // in seconds

        osMessageQueuePut(messageQueueHandle, &msg, 0, 0);

        osDelay(1000);
    }
}
```

### Receive Task — Read and Print

```c
void StartRxTask(void *argument)
{
    messageQueue_t msg;

    for (;;)
    {
        if (osMessageQueueGet(messageQueueHandle, &msg, 0, osWaitForever) == osOK)
        {
            printf("Event ID: %d, Timestamp: %lu\n",
                   msg.event_id,
                   msg.timestamp);
        }
        osDelay(1);
    }
}
```

---

## 🔗 Related Projects

- [FreeRTOS Multiple Tasks and Priorities](https://github.com/controllerstech/STM32-HAL/tree/master/freertos-cmsis/multiple-tasks-priorities)
- [FreeRTOS Configuration - Blink LED](https://github.com/controllerstech/STM32-HAL/tree/master/freertos-cmsis/setup-led-blink)

---

## 📜 License

MIT License. Free to use and modify for personal and commercial projects.
