# STM32 Custom Bootloader — Application Size & CRC Validation

Part 3 of the STM32 Custom Bootloader series. In Part 2, the bootloader validated the application using a magic number and reset handler address. This tutorial extends that validation by adding **application size and CRC32 integrity checks**. The application header stores the firmware size and CRC, allowing the bootloader to verify that the firmware image in Flash is complete and has not been corrupted before transferring execution.

## 📺 Video Tutorial

[STM32 Custom Bootloader: Application CRC & Size Validation](https://youtu.be/8-HSYugyeA8)

## 📖 Full Article

[STM32 Bootloader: Application CRC & Size Validation](https://controllerstech.com/stm32-bootloader-application-crc-size-validation/)

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

* Extends the application validation introduced in Part 2
* Stores the application size and CRC32 inside the application header
* Keeps the application header at a fixed Flash location
* Places the header in its own linker section
* Generates a binary containing only the application image
* Calculates the application size and CRC32 using a Python script
* Stores the calculated values in the application header
* Calculates the CRC32 of the application directly from Flash
* Compares the calculated CRC with the CRC stored in the header
* Verifies that the application size is within the allowed Flash region
* Verifies the application's reset handler
* Jumps to the application only when all validation checks pass

---

## How It Works

The bootloader starts after every reset and reads the application header from its fixed Flash address.

The application header contains the information required to validate the firmware:

```text
+---------------------------+
| Magic Number              |
+---------------------------+
| Application Size          |
+---------------------------+
| Application CRC32         |
+---------------------------+
| Firmware Version / ...     |
+---------------------------+
```

The bootloader performs the validation in multiple stages.

First, it checks the **magic number** to determine whether an application header is present.

Next, it reads the stored application size and verifies that the size is within the allowed application Flash region. This prevents the bootloader from calculating a CRC over an invalid or out-of-range memory area.

The bootloader then verifies the application's reset handler address to make sure that the vector table contains a valid entry point inside the application Flash region.

Finally, the bootloader calculates the **CRC32** over the application image stored in Flash. The CRC is calculated using the application start address and the exact application size stored in the header.

The calculated CRC is then compared with the CRC stored in the application header.

Only when all validation checks succeed does the bootloader transfer execution to the application.

If any check fails, the bootloader reports the error over UART and remains in the bootloader instead of executing potentially corrupted firmware.

### CRC Calculation

The CRC is calculated over the application image only.

The application header itself is **not included** in the CRC calculation. This is important because the header contains the CRC value itself.

The validation therefore follows this concept:

```text
Application Header
        |
        |  Magic
        |  Size
        |  CRC
        |
        v
Application Image
        |
        | <---- CRC calculated over this region
        v
Bootloader compares:
Calculated CRC == Stored CRC
```

---

## Generating the Application Image

The application uses a dedicated linker section for the header. Because the header contains metadata such as the application size and CRC, the application binary used for CRC calculation must exclude this header.

A post-build command is used to generate a binary containing the application image without the `.header` section.

The generated binary is then processed by the Python script.

The script determines:

* Application image size
* CRC32 of the application image

These values are subsequently written into the application header.

This creates the following workflow:

```text
Application Build
       |
       v
     ELF
       |
       | Remove .header section
       v
 Application BIN
       |
       +----> Calculate Size
       |
       +----> Calculate CRC32
       |
       v
Application Header
       |
       v
Final Application Image
```

---

## Key Files

| File                               | What it does                                                                              |
| ---------------------------------- | ----------------------------------------------------------------------------------------- |
| `app_header.h`                     | Defines the shared application header structure and metadata                              |
| `main.c` (Bootloader)              | Reads the header, validates the application, calculates CRC, and jumps to the application |
| `main.c` (Application)             | Defines the application header and initializes the application                            |
| `STM32xxxx_FLASH.ld` (Application) | Places the application header in its dedicated Flash section                              |
| `system_stm32xxxx.c`               | Relocates the application's vector table                                                  |
| Python CRC script                  | Calculates the application size and CRC32 from the generated binary                       |
| Post-build command                 | Generates the application binary while excluding the header section                       |

---

## Project Structure

```text
bootloader/
├── Bootloader/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── app_header.h
│   │   └── Src/
│   │       └── main.c              ← Application validation and CRC check
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
    ├── .ioc
    └── post-build/
        └── crc32.py                ← Size and CRC calculation
```

---

## Flash Memory Layout

| Region             | Address                                                 |
| ------------------ | ------------------------------------------------------- |
| Bootloader         | `0x08000000`                                            |
| Application Header | Fixed Flash location between bootloader and application |
| Application        | `0x08004400`                                            |

The application header is stored at a known Flash address so the bootloader can locate it without parsing the application image.

The CRC calculation starts from the application start address and covers exactly the number of bytes specified by the application size stored in the header.

```text
Flash
+-----------------------------+
| Bootloader                  |
|                             |
+-----------------------------+
| Application Header          |
| Magic                       |
| Size                        |
| CRC32                       |
| Version / Metadata          |
+-----------------------------+
| Application                 |
|                             |
| Vector Table                |
| Code                        |
| Read-only data              |
|                             |
+-----------------------------+
```

---

## CubeMX Configuration

| Peripheral | Setting                             |
| ---------- | ----------------------------------- |
| Clock      | Same configuration as Part 1        |
| USART      | Enabled for bootloader debug output |
| GPIO       | User LEDs                           |
| NVIC       | Default configuration               |

This project builds on the previous bootloader projects. No additional hardware peripherals are required.

The main changes are:

* Application header metadata
* Linker script configuration
* Application binary generation
* CRC32 calculation
* Application size validation
* Bootloader CRC verification
* Post-build Python processing

---

## How to Build

1. Clone this repository.
2. Open both the Bootloader and Application projects in STM32CubeIDE.
3. Generate code if required.
4. Build the Application project.
5. Run the configured post-build command to generate the application binary.
6. Use the Python script to calculate the application size and CRC32.
7. Update the application header with the calculated values.
8. Build the final Application project.
9. Program the Application **ELF/HEX** file to the target.
10. Build the Bootloader project.
11. Program the Bootloader at `0x08000000`.
12. Reset the board.

> **Important:** The application header contains metadata that is located at a specific Flash address. When programming the application, use an **ELF or HEX file** when possible so the linker-defined Flash addresses and sections are preserved correctly.

The exact post-build sequence is important because the CRC must be calculated from the final application image and the header itself must not be included in the CRC calculation.

---

## Testing

With a valid application programmed, resetting the board produces the following sequence:

* Bootloader starts
* Reads the application header
* Validates the magic number
* Reads the application size
* Verifies that the application size is valid
* Verifies the application's reset handler
* Calculates the CRC32 of the application image
* Compares the calculated CRC with the stored CRC
* CRC matches
* Bootloader jumps to the application
* Application starts normally

### Testing CRC Failure

To test the CRC validation, modify one or more bytes inside the application image after the CRC has been calculated.

For example, changing a byte in the application Flash without updating the stored CRC will cause the bootloader's calculated CRC to differ from the CRC stored in the header.

After reset:

```text
Stored CRC     = 0xXXXXXXXX
Calculated CRC = 0xYYYYYYYY
```

The bootloader detects the mismatch, reports the CRC validation failure over UART, and remains in the bootloader.

### Testing Application Size Validation

The bootloader also checks that the application size does not exceed the available application Flash region.

An invalid or oversized application image is rejected before the CRC calculation is performed over an invalid memory range.

---

## Validation Flow

The complete application validation process can be summarized as:

```text
                 Bootloader Start
                        |
                        v
              Read Application Header
                        |
                        v
                 Magic Number OK?
                    /       \
                  NO         YES
                  |           |
                  v           v
             Stay in      Check Size
            Bootloader         |
                              v
                       Size Within Range?
                         /          \
                       NO            YES
                       |              |
                       v              v
                  Stay in       Check Reset
                 Bootloader       Handler
                                     |
                                     v
                              Reset Handler Valid?
                                /          \
                              NO            YES
                              |              |
                              v              v
                         Stay in        Calculate CRC
                        Bootloader          |
                                            v
                                      CRC Matches?
                                      /         \
                                    NO           YES
                                    |             |
                                    v             v
                               Stay in        Jump to
                              Bootloader     Application
```

---

## Important Note About CRC

CRC32 is an **integrity check**, not a security mechanism.

A matching CRC indicates that the application image has not changed relative to the CRC stored in its header. However, CRC32 does not authenticate the firmware.

If an attacker can modify the application and the header, they can also calculate a new CRC and update the header.

Therefore, this validation mechanism is useful for detecting:

* Flash corruption
* Incomplete firmware programming
* Accidental modification
* Invalid application images
* Data transmission/programming errors

For protection against deliberately modified or unauthorized firmware, a cryptographic authentication mechanism such as a digital signature or MAC is required.

---

## Series

| Part       | Topic                                                | Link                                                                          |
| ---------- | ---------------------------------------------------- | ----------------------------------------------------------------------------- |
| Part 1     | Flash Layout & Jump to Application                   | https://controllerstech.com/stm32-custom-bootloader-tutorial/                 |
| Part 2     | Application Validation Using Magic Number            | https://controllerstech.com/stm32-bootloader-application-validation/          |
| **Part 3** | **Application Size & CRC Validation (this project)** | https://controllerstech.com/stm32-bootloader-application-crc-size-validation/ |

---

## Previous Part

In Part 2, the bootloader introduced a dedicated application header containing a magic number. The bootloader used the magic number and reset handler address to determine whether a valid application was present before jumping to it.

**Part 2:** STM32 Custom Bootloader: Application Validation Using Magic Number

https://controllerstech.com/stm32-bootloader-application-validation/

---

## Next Part

In the next tutorial, we will build on the application validation mechanism and look at how the bootloader can provide stronger protection for the firmware. The CRC32 check used in this project detects accidental corruption, but it cannot prevent an attacker from modifying the firmware and recalculating the CRC.

The next stage is to introduce **cryptographic firmware authentication**, allowing the bootloader to verify that the firmware was generated by a trusted source before executing it.

**Part 4:** STM32 Custom Bootloader: Secure Firmware Authentication

---

## License

Open source — free to use and modify. If this project helped you, consider supporting the work:

https://paypal.me/controllertech
