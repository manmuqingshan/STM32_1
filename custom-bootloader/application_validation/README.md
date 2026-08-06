# STM32 Custom Bootloader — Application Validation Using Magic Number

Part 2 of the STM32 Custom Bootloader series. In Part 1, the bootloader always transferred execution to the application after reset. This tutorial improves that design by validating the application before the jump. A dedicated application header containing a magic number is stored at a fixed Flash location, allowing the bootloader to determine whether a valid firmware image exists. If validation succeeds, the bootloader jumps to the application; otherwise, it remains in the bootloader and reports the error.

## 📺 Video Tutorial

[STM32 Custom Bootloader: Application Validation Using Magic Number](https://youtu.be/w1NorigAWJA)

## 📖 Full Article

[STM32 Bootloader: Application Validation Using Magic Number](https://controllerstech.com/stm32-bootloader-application-validation/)

---

## Hardware Used

| Component | Details                                                                                            |
| --------- | -------------------------------------------------------------------------------------------------- |
| Board     | STM32 Nucleo L496ZG *(any STM32 with sufficient Flash can be used by adjusting the memory layout)* |
| UART      | ST-Link Virtual COM Port                                                                           |
| LEDs      | Onboard User LEDs                                                                                  |

### Pin Assignment

| Signal    | Pin         | Function                             |
| --------- | ----------- | ------------------------------------ |
| UART TX   | ST-Link VCP | Bootloader debug messages            |
| UART RX   | ST-Link VCP | Optional                             |
| User LED  | Onboard     | Indicates the application is running |
| Error LED | Onboard     | Indicates validation failure         |

> No additional hardware is required. The entire project runs using the onboard peripherals of the STM32 development board.

---

## What This Project Does

* Introduces a dedicated application header stored at a fixed Flash address
* Defines a common `app_header_t` structure shared between the bootloader and the application
* Stores a magic number inside the application header
* Places the header in its own linker section
* Modifies the application's linker script to reserve space for the header
* Implements application validation inside the bootloader
* Verifies both the magic number and the application's reset handler
* Jumps to the application only if all validation checks pass

---

## How It Works

The bootloader starts after every reset and reads the application header from its fixed Flash address. The first validation step checks whether the stored magic number matches the expected value.

If the magic number is valid, the bootloader performs a second sanity check by reading the application's reset handler from the vector table and verifying that it points into the application's Flash region. This prevents accidental jumps to erased or invalid memory.

Only when both checks succeed does the bootloader transfer execution to the application. Otherwise, it reports the validation failure and remains in the bootloader.

The application now contains an application header placed in a dedicated linker section. Although only the magic number is used in this tutorial, the header also reserves fields for the application size, CRC, and firmware version, which will be used in later parts of the series.

### Key Files

| File                               | What it does                                                                       |
| ---------------------------------- | ---------------------------------------------------------------------------------- |
| `app_header.h`                     | Defines the shared application header structure and magic number                   |
| `main.c` (Bootloader)              | Reads the application header, validates the firmware, and jumps to the application |
| `main.c` (Application)             | Creates the application header and initializes the application                     |
| `STM32xxxx_FLASH.ld` (Application) | Places the application header at a fixed Flash address                             |
| `system_stm32xxxx.c`               | Relocates the application's vector table                                           |

---

## Project Structure

```text
bootloader/
├── Bootloader/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── app_header.h
│   │   └── Src/
│   │       └── main.c              ← Application validation
│   ├── STM32xxxx_FLASH.ld
│   └── .ioc
│
└── Application/
    ├── Core/
    │   ├── Inc/
    │   │   └── app_header.h
    │   └── Src/
    │       └── main.c              ← Application header definition
    ├── STM32xxxx_FLASH.ld          ← Header placed in dedicated section
    └── .ioc
```

---

## Flash Memory Layout

| Region             | Address                                                 |
| ------------------ | ------------------------------------------------------- |
| Bootloader         | `0x08000000`                                            |
| Application Header | Fixed Flash location between bootloader and application |
| Application        | `0x08004400`                                            |

The application header is stored separately from the application code so the bootloader can always locate it without parsing the firmware image.

---

## CubeMX Configuration

| Peripheral | Setting                             |
| ---------- | ----------------------------------- |
| Clock      | Same configuration as Part 1        |
| USART      | Enabled for bootloader debug output |
| GPIO       | User LEDs                           |
| NVIC       | Default configuration               |

This project builds directly on **Part 1**. No new peripherals are added. The primary changes are the application header, linker script modifications, and bootloader validation logic.

---

## How to Build

1. Clone this repository.
2. Open both the Bootloader and Application projects in STM32CubeIDE.
3. Generate code if required.
4. Build the Application project.
5. Program the Application **ELF** file to the target.
6. Build the Bootloader project.
7. Program the Bootloader binary at `0x08000000`.
8. Reset the board.

> **Important:** Flash the **ELF** (or HEX) file for the application rather than the BIN file. The application header is stored in a dedicated linker section, and only ELF/HEX files preserve the section addresses correctly.

---

## Testing

With a valid application programmed, resetting the board produces the following sequence:

* Bootloader starts
* Reads the application header
* Validates the magic number
* Verifies the application's reset handler
* Jumps to the application
* Application starts normally

To test the validation logic, change the application's magic number to an incorrect value, rebuild, and reprogram the application.

After reset, the bootloader detects the invalid firmware, reports the validation failure over UART, and remains in the bootloader instead of jumping to the application.

---

## Series

| Part       | Topic                                                        | Link                                                                 |
| ---------- | ------------------------------------------------------------ | -------------------------------------------------------------------- |
| Part 1     | Flash Layout & Jump to Application                           | https://controllerstech.com/stm32-custom-bootloader-tutorial/        |
| **Part 2** | **Application Validation Using Magic Number (this project)** | https://controllerstech.com/stm32-bootloader-application-validation/ |

---

## Next Part

In the next tutorial, we strengthen the bootloader by adding **application size** and **CRC32 integrity validation**. The application stores its size and CRC inside the application header, while the bootloader calculates the CRC over the application image in Flash and compares it with the stored value. Together with the existing magic number and reset handler checks, this ensures that only complete and uncorrupted firmware is executed. The tutorial also demonstrates the post-build process for generating a CRC-ready application binary and a Python script to calculate the application size and CRC before programming the device.

**Part 3:** STM32 Custom Bootloader: Application Size & CRC Validation

https://controllerstech.com/stm32-bootloader-application-crc-size-validation/

---

## License

Open source — free to use and modify. If this project helped you, consider supporting the work:

https://paypal.me/controllertech
