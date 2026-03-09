# STM32 Low Power — Standby Mode

This project demonstrates how to enter and exit **Standby Mode** on the STM32 using the HAL library in STM32CubeIDE.

Standby mode provides the lowest possible power consumption. The 1.2V domain is powered off entirely — SRAM and peripheral states are lost. Waking from Standby is equivalent to a system reset; execution restarts from the beginning of `main()`. Only the RTC, backup registers, and VBAT-powered circuitry retain their state.

This is part of the STM32 Low Power Modes series on [Controllerstech](https://controllerstech.com/low-power-modes-in-stm32/).

---

## Hardware

- **Board:** STM32 (e.g. STM32F446RE / STM32F103C8)
- **Wake-up sources used:**
  - Wake-up pin (PA0 / WKUP pin — rising edge)
  - RTC Wake-up Timer (~5 seconds)
- **LED:** PA5 — used to indicate standby/wake state

---

## How Standby Mode Works

| Property | Value |
|---|---|
| **CPU** | Powered off |
| **Regulator** | OFF (1.2V domain powered down) |
| **Clocks** | All OFF except LSI/LSE if RTC is enabled |
| **SRAM / Registers** | Lost (Backup domain + RTC retained) |
| **Current Draw (typical)** | ~1–5 µA |
| **Wake-up time** | Slow (full reset sequence) |
| **Reset on wake-up** | Yes — code restarts from `main()` |

---

## Entering Standby Mode

Wake-up flags must be cleared before entering to avoid an immediate wake-up:

```c
__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
__HAL_RTC_WAKEUPTIMER_CLEAR_FLAG(&hrtc, RTC_FLAG_WUTF);
```

Configure wake-up sources, then enter Standby:

```c
HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);

// RTC wake-up ~5s: WakeUpCounter = 5s / 0.0005s = 10000 = 0x2710
HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0x2710, RTC_WAKEUPCLOCK_RTCCLK_DIV16);

HAL_PWR_EnterSTANDBYMode();
```

---

## Detecting Wake-up from Standby

Since wake-up is a reset, check the SBF flag at the start of `main()`:

```c
if (__HAL_PWR_GET_FLAG(PWR_FLAG_SB) != RESET)
{
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);  // Clear the Standby flag

    // MCU has just woken from Standby — handle accordingly
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    HAL_RTCEx_DeactivateWakeUpTimer(&hrtc);
}
```

---

## Wake-up Sources

| Source | Description |
|---|---|
| **Wake-up pin (PA0)** | Rising edge on WKUP pin |
| **RTC Wake-up Timer** | Periodic timer interrupt |
| **RTC Alarm A / B** | Alarm-triggered wake-up |
| **NRST pin** | Hardware reset |
| **IWDG** | Independent watchdog timeout |

---

## Key Notes

- SRAM and peripheral states are **not retained** — unlike Stop mode.
- Always check the **SBF flag** on every reset to determine if the MCU woke from Standby.
- Use **RTC backup registers** to preserve critical data across Standby cycles.
- The `while(1)` after `HAL_PWR_EnterSTANDBYMode()` is never reached — it is there only as a safety construct.

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
- 📁 [Stop Mode Project](../stop-mode/)

---

## License

MIT License. Free to use and modify for personal and commercial projects.
