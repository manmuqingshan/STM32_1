# STM32 Low Power — Stop Mode

This project demonstrates how to enter and exit **Stop Mode** on the STM32 using the HAL library in STM32CubeIDE.

In Stop mode, all clocks in the 1.2V domain are halted (HSI, HSE, PLLs), while SRAM and register contents are preserved. The low-power regulator remains active. This mode offers significantly lower power consumption than Sleep mode while still retaining system state.

This is part of the STM32 Low Power Modes series on [Controllerstech](https://controllerstech.com/low-power-modes-in-stm32/).

---

## Hardware

- **Board:** STM32 (e.g. STM32F446RE / STM32F103C8)
- **Wake-up sources used:**
  - RTC Wake-up Timer (periodic, ~10 seconds)
  - External interrupt (EXTI) via user button (GPIO pin 13)
- **LED:** PA5 — used to indicate stop/wake state

---

## How Stop Mode Works

| Property | Value |
|---|---|
| **CPU** | Stopped |
| **Clocks** | HSI / HSE / PLL OFF; LSI / LSE / RTC can stay ON |
| **SRAM / Registers** | Retained |
| **Current Draw (typical)** | ~20–100 µA |
| **Wake-up time** | Moderate |
| **Reset on wake-up** | No (but clocks must be reconfigured) |

---

## Entering Stop Mode

SysTick is suspended first, then the MCU enters Stop mode using the low-power regulator:

```c
HAL_SuspendTick();
HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);
```

---

## Wake-up Sources

### RTC Wake-up Timer
```c
void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc)
{
    SystemClock_Config();   // Must reconfigure clocks after Stop mode
    HAL_ResumeTick();
    // SleepOnExit active — MCU goes back to Stop mode after this ISR
}
```

### External Interrupt (Button on pin 13)
```c
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == GPIO_PIN_13)
    {
        SystemClock_Config();
        HAL_ResumeTick();
        HAL_PWR_DisableSleepOnExit();  // Return to main loop
    }
}
```

> ⚠️ **Important:** After waking from Stop mode, system clocks are not automatically restored. Always call `SystemClock_Config()` inside the wake-up callback before using any peripherals.

---

## RTC Wake-up Timer Configuration

The RTC wake-up timer is configured for approximately 10 seconds:

```c
// Wake-up Time Base = 16 / 32KHz = 0.0005s
// WakeUpCounter = 10s / 0.0005s = 20000 = 0x4E20
HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0x4E20, RTC_WAKEUPCLOCK_RTCCLK_DIV16);
```

---

## SLEEPONEXIT Feature

`HAL_PWR_EnableSleepOnExit()` makes the MCU automatically re-enter Stop mode after each ISR exits, without returning to the main loop.

- Woken by **RTC** → ISR runs → goes back to Stop mode (SLEEPONEXIT active)
- Woken by **EXTI** → ISR runs → returns to main loop (SLEEPONEXIT disabled inside ISR)

---

## Project Setup

1. Clone or download this repository
2. Open **STM32CubeIDE**
3. Go to **File → Import → Existing Projects into Workspace**
4. Select this folder and click **Finish**
5. Build and flash to your STM32 board

---

## Related Resources

- 📖 [Full Tutorial — Controllerstech](https://controllerstech.com/low-power-modes-in-stm32/)
- 📁 [Sleep Mode Project](../sleep-mode/)
- 📁 [Standby Mode Project](../standby-mode/)

---

## License

MIT License. Free to use and modify for personal and commercial projects.
