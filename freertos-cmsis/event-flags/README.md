# STM32 FreeRTOS Event Flags

This project demonstrates how to use event flags in FreeRTOS on STM32 using CMSIS-OS v2.
We use a single event group to synchronize multiple tasks, demonstrate waiting for ALL and ANY
events, and trigger tasks directly from ISR callbacks using a button and UART.

---

## 📺 Tutorial

**[Watch the full video tutorial on YouTube →](https://youtu.be/R0HMTY7PyQ4)**

**[Read the full written guide →](https://controllerstech.com/stm32-freertos-event-flags/)**

---

## 📋 Overview

Semaphores and mutexes can only represent a single condition at a time. Event flags solve
this by letting you track multiple events in a single 32-bit value, where each bit represents
a different event. A task can then wait for any one of those events or wait until all of them
are set before proceeding.

This project covers three scenarios:

- **Wait for ALL events** — Processing task stays blocked until all three sensor tasks have set their flags
- **Wait for ANY event** — Processing task unblocks as soon as any one of the three tasks fires, and identifies which one triggered it
- **ISR-driven events** — A button press and a UART receive each set their own flag from inside an interrupt callback, waking their respective tasks

---

## 🛠 Hardware

- STM32L496 Nucleo board (any STM32 Nucleo board works with minor pin changes)
- USB cable for Virtual COM port (ST-LINK)
- Push button connected between PA3 and GND
- LED on PB7

---

## ⚙️ CubeMX Configuration

| Parameter | Value |
|-----------|-------|
| RTOS | FreeRTOS (CMSIS-OS v2) |
| Heap Size | 10 KB |
| Event Group | `myEvent01Handle` — Dynamic allocation |
| UART | LPUART1 — PG7 (TX), PG8 (RX) |
| Baud Rate | 115200, 8N1 |
| Button Pin | PA3 — GPIO_EXTI3, Pull-up |
| LED Pin | PB7 — GPIO Output |

### Task Configuration

| Task | Priority | Stack Size | Role |
|------|----------|------------|------|
| StartTask1 | Normal | 512 words | Sets TASK1_EVENT every 3000ms |
| StartTask2 | Normal | 512 words | Sets TASK2_EVENT every 5000ms |
| StartTask3 | Normal | 512 words | Sets TASK3_EVENT every 7000ms |
| StartTask4 | Normal | 512 words | Waits for events and processes data |
| StartButtonTask | Normal | 128 words | Waits for BUTTON_PRESSED flag, toggles LED |
| StartUartTask | Normal | 512 words | Waits for UART_RECEIVED flag, prints data |

---

## 💻 Task Code

### Event Bit Definitions
```c
#define TASK1_EVENT      (1 << 0)
#define TASK2_EVENT      (1 << 1)
#define TASK3_EVENT      (1 << 2)
#define BUTTON_PRESSED   (1 << 3)
#define UART_RECEIVED    (1 << 4)

uint8_t RxData[10];
```

### Wait for ALL Events
```c
void StartTask1(void *argument)
{
    for(;;)
    {
        printf("Task1 Sending Event\n");
        osEventFlagsSet(myEvent01Handle, TASK1_EVENT);
        osDelay(3000);
    }
}

void StartTask2(void *argument)
{
    for(;;)
    {
        printf("Task2 Sending Event\n");
        osEventFlagsSet(myEvent01Handle, TASK2_EVENT);
        osDelay(5000);
    }
}

void StartTask3(void *argument)
{
    for(;;)
    {
        printf("Task3 Sending Event\n");
        osEventFlagsSet(myEvent01Handle, TASK3_EVENT);
        osDelay(7000);
    }
}

void StartTask4(void *argument)
{
    for(;;)
    {
        printf("Processing Task: Waiting for all sensors...\n");
        osEventFlagsWait(myEvent01Handle,
                         TASK1_EVENT | TASK2_EVENT | TASK3_EVENT,
                         osFlagsWaitAll,
                         osWaitForever);
        printf("Processing Task: All sensors ready! Processing data...\n\n");
        osDelay(500);
    }
}
```

### Wait for ANY Event
```c
void StartTask4(void *argument)
{
    uint32_t flags;

    for(;;)
    {
        printf("Processing Task: Waiting for any Event...\n");
        flags = osEventFlagsWait(myEvent01Handle,
                                 TASK1_EVENT | TASK2_EVENT | TASK3_EVENT,
                                 osFlagsWaitAny,
                                 osWaitForever);

        if (flags & TASK1_EVENT) printf("Processing Task: Event received from Task1\n");
        if (flags & TASK2_EVENT) printf("Processing Task: Event received from Task2\n");
        if (flags & TASK3_EVENT) printf("Processing Task: Event received from Task3\n");

        osDelay(500);
    }
}
```

### ISR Callbacks
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    osEventFlagsSet(myEvent01Handle, BUTTON_PRESSED);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    osEventFlagsSet(myEvent01Handle, UART_RECEIVED);
    HAL_UART_Receive_IT(&hlpuart1, RxData, 5);
}
```

### Button and UART Tasks
```c
void StartButtonTask(void *argument)
{
    for(;;)
    {
        osEventFlagsWait(myEvent01Handle, BUTTON_PRESSED, osFlagsWaitAll, osWaitForever);
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
        osDelay(500);
    }
}

void StartUartTask(void *argument)
{
    for(;;)
    {
        osEventFlagsWait(myEvent01Handle, UART_RECEIVED, osFlagsWaitAll, osWaitForever);
        printf("Data Received: %s\n", RxData);
    }
}
```

---

## 🔗 Related Projects

* [FreeRTOS Stack Management and Overflow Detection](https://github.com/controllerstech/STM32-HAL/tree/master/freertos-cmsis/Stack-Management)
* [FreeRTOS Software Timers (Periodic & One-Shot)](https://github.com/controllerstech/STM32-FreeRTOS-Software-Timers)
* [FreeRTOS Mutex — Priority Inheritance and Recursive Mutex](https://github.com/controllerstech/STM32-FreeRTOS-Mutex)
* [FreeRTOS Semaphores — Binary and Counting](https://github.com/controllerstech/STM32-HAL/tree/master/freertos-cmsis/semaphores-binary-counting)
* [FreeRTOS Queue — Inter-Task Communication](https://github.com/controllerstech/STM32-HAL/tree/master/freertos-cmsis/queue-inter-task-communication)
* [FreeRTOS Multiple Tasks and Priorities](https://github.com/controllerstech/STM32-HAL/tree/master/freertos-cmsis/multiple-tasks-priorities)
* [FreeRTOS Setup and Blink LED](https://github.com/controllerstech/STM32-HAL/tree/master/freertos-cmsis/setup-led-blink)

---

## 📜 License

MIT License. Free to use and modify for personal and commercial projects.
