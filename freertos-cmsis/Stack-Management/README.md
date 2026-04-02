# STM32 FreeRTOS Stack Management (Monitor & Overflow Detection)

This project demonstrates how to monitor FreeRTOS task stack usage and detect stack overflows in STM32 using CMSIS-OS v2.
We create a dedicated monitor task that tracks stack consumption, enable stack overflow detection in CubeMX, and implement an overflow hook to catch critical errors before they cause system crashes.

---

## 📺 Tutorial

**[Watch the full video tutorial on YouTube →](https://youtu.be/PY_GuphBsyc)**

**[Read the full written guide →](https://controllerstech.com/stm32-freertos-stack-management-monitor-detect-overflow/)**

---

## 📋 Overview

Stack overflow is one of the most difficult problems to debug in FreeRTOS systems. Silent data corruption and unpredictable crashes make it impossible to trace the root cause. This project teaches you to monitor stack usage continuously and detect overflows automatically.

This project covers:

- **Stack Monitoring Task** — Continuously tracks free stack space in all tasks
- **High Water Mark Measurement** — Records worst-case stack usage for each task
- **Printf Stack Consumption** — Measures actual memory used by printf() function
- **Stack Overflow Detection** — Enables automatic overflow hook in CubeMX
- **Overflow Hook Implementation** — Captures overflow events and logs the problematic task
- **Dynamic Stack Testing** — Tests stack behavior with arrays and function calls

---

## 🛠 Hardware

- STM32L496 Nucleo board (any STM32 Nucleo board works with minor changes)
- USB cable for Virtual COM port (ST-LINK)
- Serial terminal application (PuTTY, Tera Term, or Arduino Serial Monitor)

---

## ⚙️ CubeMX Configuration

| Parameter | Value |
|----------|------|
| RTOS | FreeRTOS (CMSIS-OS v2) |
| Heap Size | 10 KB |
| Task 1 | `Task1` — Normal Priority, Stack: 128 words |
| Task 2 | `Task2` — Normal Priority, Stack: 256 words |
| Task 3 | `Task3` — Normal Priority, Stack: 512 words |
| Monitor Task | `MonitorTask` — Low Priority, Stack: 512 words |
| Stack Overflow Hook | Option 2 — Pattern-based detection |
| UART | LPUART1 — PG7 (TX), PG8 (RX) |
| Baud Rate | 115200, 8N1 |

### Task Configuration

| Task | Priority | Stack Size | Purpose |
|------|----------|-----------|---------|
| Task1 | Normal | 128 words | Test application task |
| Task2 | Normal | 256 words | Test application task |
| Task3 | Normal | 512 words | Test application task |
| MonitorTask | Low | 512 words | Monitor stack usage of all tasks |

---

## 💻 Code

### Monitor Task Implementation
```c
void StartmonitorTask(void *argument)
{
  uint32_t task1Stack, task2Stack, task3Stack;
  
  for(;;)
  {
    osDelay(5000);
    
    task1Stack = osThreadGetStackSpace(Task1Handle);
    task2Stack = osThreadGetStackSpace(Task2Handle);
    task3Stack = osThreadGetStackSpace(Task3Handle);
    
    printf("Stack Free - Task1: %lu Bytes\t Task2: %lu Bytes\t Task3: %lu Bytes\n", 
           task1Stack, task2Stack, task3Stack);
  }
}
```

---

### Task with Printf (Measures Stack Consumption)
```c
void StartTask1(void *argument)
{
  for(;;)
  {
	  printf ("Hello from Task 1\n");
    osDelay(2000);
  }
}
```

---

### Task with Local Array (Tests Overflow)
```c
void StartTask1(void *argument)
{
  uint8_t foo[100];
  
  for(;;)
  {
	  printf ("Hello from Task 1\n");
    osDelay(2000);
  }
}
```

---

### Stack Overflow Hook
```c
extern UART_HandleTypeDef hlpuart1;

void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName)
{
    HAL_UART_DeInit(&hlpuart1);
    HAL_UART_Init(&hlpuart1);
    
	  char msg[100];
	  int len = sprintf (msg, "Stack Overflow Detected in: %s\n", pcTaskName);
	  HAL_UART_Transmit(&hlpuart1, (uint8_t *)msg, len, 1000);
    
    while(1);
}
```

---

## 🔁 Key Behavior

* Monitor task runs every 5 seconds
* Reads free stack space from Task1, Task2, and Task3 using `osThreadGetStackSpace()`
* Prints measurements to serial console
* Printf function consumes ~150 bytes of stack
* Local arrays consume their size plus compiler overhead
* Stack overflow hook activates when overflow is detected
* System halts to prevent further corruption

---

## ⚠️ Important Notes

* `osThreadGetStackSpace()` returns the high water mark (minimum free space ever recorded)
* High water mark never increases, only stays same or decreases
* Printf() uses same stack space regardless of string length due to internal buffers
* Stack overflow detection Option 2 is more reliable than Option 1
* Do not use printf() inside the overflow hook — use direct UART transmission instead
* Monitor task should have low priority to avoid interfering with application tasks
* Allocate larger stack to monitor task (512+ words) to accommodate printf()

---

## 🔗 Related Projects

* [FreeRTOS Software Timers (Periodic & One-Shot)](https://github.com/controllerstech/STM32-FreeRTOS-Software-Timers)
* [FreeRTOS Event Flags](https://github.com/controllerstech/STM32-FreeRTOS-Event-Flags)
* [FreeRTOS Mutex — Priority Inheritance and Recursive Mutex](https://github.com/controllerstech/STM32-FreeRTOS-Mutex)
* [FreeRTOS Semaphores — Binary and Counting](https://github.com/controllerstech/STM32-FreeRTOS-Semaphores)
* [FreeRTOS Queue — Inter-Task Communication](https://github.com/controllerstech/STM32-FreeRTOS-Queue)
* [FreeRTOS Multiple Tasks and Priorities](https://github.com/controllerstech/STM32-FreeRTOS-Multiple-Tasks)
* [FreeRTOS Setup and Blink LED](https://github.com/controllerstech/STM32-HAL/tree/master/freertos-cmsis/setup-led-blink)

---

## 📜 License

MIT License. Free to use and modify for personal and commercial projects.
