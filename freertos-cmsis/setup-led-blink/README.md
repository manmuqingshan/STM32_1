# STM32 FreeRTOS – Setup & LED Blink

This is **Part 1** of the STM32 FreeRTOS (CMSIS-RTOS V2) series.

We configure FreeRTOS on STM32 using CMSIS-RTOS V2 and blink an LED from a FreeRTOS task using `osDelay()` instead of `HAL_Delay()`.

---

## 📺 Tutorial

**[Watch the full video tutorial on YouTube →](https://youtu.be/siQTCJ2t5v4)**

**[Read the full written guide →](https://controllerstech.com/stm32-freertos-tutorial-cubemx-led-example/)**

---

## Key Points

- Change HAL time base from **SysTick to TIM6** — required when using FreeRTOS
- Enable **newlib reentrant** in FreeRTOS Advanced Settings
- Always use `osDelay()` inside tasks, not `HAL_Delay()`
- Code after `osKernelStart()` will never execute — put all logic inside tasks

---

## Task Code

```c
void StartDefaultTask(void *argument)
{
    for (;;)
    {
        HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_7);
        osDelay(500);
    }
}
```

---

## Result

The image below shows the LED blinking:

![Result](images/result.gif)

---

## Full Tutorial

👉 https://controllerstech.com/stm32-freertos-tutorial-cubemx-led-example/

---

## Related Tutorials

| Part | Topic | Link |
|------|-------|------|
| Part 2 | Coming soon | – |

---

## 🔗 Related Projects

* [FreeRTOS Stack Management and Overflow Detection](https://github.com/controllerstech/STM32-HAL/tree/master/freertos-cmsis/Stack-Management)
* [FreeRTOS Software Timers (Periodic & One-Shot)](https://github.com/controllerstech/STM32-FreeRTOS-Software-Timers)
* [FreeRTOS Event Flags](https://github.com/controllerstech/STM32-FreeRTOS-Event-Flags)
* [FreeRTOS Mutex — Priority Inheritance and Recursive Mutex](https://github.com/controllerstech/STM32-FreeRTOS-Mutex)
* [FreeRTOS Semaphores — Binary and Counting](https://github.com/controllerstech/STM32-FreeRTOS-Semaphores)
* [FreeRTOS Queue — Inter-Task Communication](https://github.com/controllerstech/STM32-FreeRTOS-Queue)
* [FreeRTOS Multiple Tasks and Priorities](https://github.com/controllerstech/STM32-FreeRTOS-Multiple-Tasks)

---

## License

This example is provided for educational purposes under Controllerstech Guidelines.
