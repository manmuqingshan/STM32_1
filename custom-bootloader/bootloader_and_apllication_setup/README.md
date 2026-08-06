# STM32 Custom Bootloader — Flash Layout & Jump to Application

Part 1 of the STM32 Custom Bootloader series. This tutorial lays the foundation for the entire series by creating a minimal bootloader capable of transferring execution to a separate application stored at a different location in Flash memory. The bootloader and application are developed as two independent STM32CubeIDE projects, each with its own linker configuration, allowing future tutorials to add firmware validation, CRC checking, and OTA updates without changing the overall architecture.

## 📺 Video Tutorial

[STM32 Custom Bootloader: Flash Layout & Jump to Application](https://youtu.be/fRrAiKU7FzU)

## 📖 Full Article

[STM32 Custom Bootloader Tutorial: Flash Layout and Jump to Application](https://controllerstech.com/stm32-custom-bootloader-tutorial/)

---

## Hardware Used

| Component | Details                                                                                              |
| --------- | ---------------------------------------------------------------------------------------------------- |
| Board     | STM32 Nucleo L496ZG *(any STM32 with sufficient Flash can be used with appropriate address changes)* |
| UART      | ST-Link Virtual COM Port                                                                             |
| LED       | Onboard User LED                                                                                     |

### Pin Assignment

| Signal   | Pin            | Function                         |
| -------- | -------------- | -------------------------------- |
| UART TX  | ST-Link VCP_RX | Debug messages                   |
| User LED | Onboard        | Indicates application is running |

> No external hardware is required. The entire tutorial uses only the onboard peripherals of the STM32 development board.

---

## What This Project Does

* Creates two independent STM32CubeIDE projects:

  * Bootloader
  * Application
* Reserves the first 16 KB of Flash for the bootloader
* Relocates the application to a higher Flash address
* Modifies the linker script for both projects
* Relocates the application's interrupt vector table
* Implements the bootloader jump routine
* Builds separate binary files for the bootloader and application
* Programs each binary at its corresponding Flash address using STM32CubeProgrammer

---

## How It Works

After every reset, the STM32 always begins execution from the start of Flash memory, where the bootloader resides. The bootloader performs its initialization and then transfers execution to the application located at a different Flash address.

The jump routine first disables interrupts, stops the SysTick timer, and deinitializes the HAL so that no peripherals remain active. It then reads the application's initial stack pointer and reset handler from the application's vector table, loads the Main Stack Pointer (MSP), and branches to the application's reset handler.

Since the application no longer starts at `0x08000000`, its linker script is modified so that all code and data are linked relative to the new Flash origin. The application's vector table is also relocated by updating the `VTOR` register before interrupts are enabled.

This separation allows the bootloader and application to be updated independently while sharing the same microcontroller.

### Key Files

| File                               | What it does                                                         |
| ---------------------------------- | -------------------------------------------------------------------- |
| `main.c` (Bootloader)              | Initializes the bootloader and performs the jump to the application  |
| `main.c` (Application)             | Initializes the application after the bootloader transfers execution |
| `STM32xxxx_FLASH.ld` (Bootloader)  | Reserves the first portion of Flash for the bootloader               |
| `STM32xxxx_FLASH.ld` (Application) | Relocates the application's Flash origin                             |
| `system_stm32xxxx.c`               | Relocates the vector table using `SCB->VTOR`                         |

---

## Project Structure

```text
bootloader/
├── Bootloader/
│   ├── Core/
│   │   ├── Inc/
│   │   └── Src/
│   │       └── main.c          ← Bootloader jump routine
│   ├── STM32xxxx_FLASH.ld      ← Bootloader linker script
│   └── .ioc
│
└── Application/
    ├── Core/
    │   ├── Inc/
    │   └── Src/
    │       └── main.c          ← Application entry point
    ├── STM32xxxx_FLASH.ld      ← Application linker script
    └── .ioc
```

---

## Flash Memory Layout

| Region      | Address                    |
| ----------- | -------------------------- |
| Bootloader  | `0x08000000`               |
| Reserved    | `0x08004000` *(alignment)* |
| Application | `0x08004400`               |

The application is linked to start from `0x08004400`, while the bootloader occupies the beginning of Flash.

---

## CubeMX Configuration

| Peripheral | Setting                      |
| ---------- | ---------------------------- |
| Clock      | Default CubeMX configuration |
| USART      | Enabled for debugging        |
| GPIO       | User LED                     |
| NVIC       | Default configuration        |

**Two independent CubeMX projects** are created in this tutorial—one for the bootloader and one for the application. The only significant difference is the Flash memory layout defined in their respective linker scripts.

---

## How to Build

1. Clone this repository.
2. Open the Bootloader project in STM32CubeIDE.
3. Generate code if required.
4. Build the Bootloader project.
5. Open the Application project.
6. Modify the linker script so the application starts at `0x08004400`.
7. Build the Application project.
8. Open STM32CubeProgrammer.
9. Flash the Bootloader binary at `0x08000000`.
10. Flash the Application binary at `0x08004400`.
11. Reset the board.

---

## Testing

After reset, the bootloader executes first and immediately transfers control to the application.

Open a serial terminal (if UART messages are enabled) to verify that both projects execute in sequence.

The application should start normally, initialize its peripherals, and begin blinking the onboard LED.

If the application does not start:

* Verify that the application was programmed at `0x08004400`.
* Ensure the linker script uses the correct Flash origin.
* Confirm that the application's vector table has been relocated correctly.
* Check that the bootloader loads the MSP before branching to the reset handler.

---

## Next Part

In the next tutorial, we add **application validation** to the bootloader using a **magic number** stored in a dedicated application header. Before jumping to the application, the bootloader verifies that a valid firmware image is present by checking the magic number and performing a basic reset handler sanity check. This prevents the bootloader from executing an invalid or corrupted application and forms the foundation for more advanced validation methods such as application size and CRC checking.

**Part 2:** STM32 Custom Bootloader: Application Validation Using Magic Number

https://controllerstech.com/stm32-bootloader-application-validation/

---

## License

Open source — free to use and modify. If this project helped you, consider supporting the work:

https://paypal.me/controllertech
