# STM32 Custom Bootloader — Full OTA Firmware Update

Part 5 of the STM32 Custom Bootloader series. In the previous part, the bootloader was extended with an OTA FLAG mechanism that allows the application to request an update. This tutorial takes the next step and implements the actual OTA firmware update process.

The bootloader receives an OTA image containing a small metadata header followed by the application firmware. It validates the image information, erases the application Flash area, programs the new firmware, calculates the CRC while writing, and finally verifies the complete image before allowing the application to run.

The OTA input is handled through a simple stream interface, making it possible to use different firmware sources without changing the core OTA update logic.

---

## 📺 Video Tutorial

[STM32 OTA Bootloader: Complete Firmware Update](https://youtu.be/iwxWuUbizVA)

---

## 📖 Full Article

[STM32 OTA Bootloader: Full Firmware Update](https://controllerstech.com/stm32-ota-bootloader-full-update/)

---

## What This Project Does

* Extends the STM32 custom bootloader with a complete OTA firmware update process
* Creates an OTA image containing firmware metadata and application data
* Stores the application size, CRC32, magic number, and version in the OTA image header
* Allows the bootloader to read the firmware through an OTA stream
* Validates the OTA image header before programming Flash
* Erases the application area before installing the new firmware
* Programs the application firmware into STM32 Flash in chunks
* Calculates the CRC32 while the firmware is being written
* Compares the calculated CRC with the CRC stored in the OTA image
* Validates the new application before starting it
* Uses the existing application validation mechanism from the previous tutorials
* Keeps the OTA update engine independent from the actual firmware source

---

## How It Works

The OTA update starts when the bootloader enters the update flow. The firmware is prepared as an OTA image containing a metadata header followed by the application binary.

The metadata allows the bootloader to know important information about the firmware before programming it.

The OTA image has the following general structure:

```text
+---------------------------+
| Magic Number              |
+---------------------------+
| Application Size          |
+---------------------------+
| Application CRC32         |
+---------------------------+
| Firmware Version          |
+---------------------------+
|                           |
|      Application          |
|       Firmware            |
|                           |
|        Binary             |
|                           |
+---------------------------+
```

The bootloader first reads the metadata header and checks that the image is valid. Once the header passes the required checks, the bootloader erases the application Flash area.

The firmware is then read from the OTA stream in chunks and written into the application area. While the data is being programmed, the bootloader also calculates the CRC32 of the received firmware.

After the complete firmware has been written, the calculated CRC is compared with the CRC stored in the OTA image header.

Only when the CRC matches is the new application considered valid.

The basic flow is:

```text
OTA Image
    |
    v
Read OTA Header
    |
    v
Validate Metadata
    |
    +----------------------+
    |                      |
  Invalid                Valid
    |                      |
    v                      v
  Abort              Erase Application
                           |
                           v
                    Read Firmware
                           |
                           v
                    Program Flash
                           |
                           v
                    Calculate CRC
                           |
                           v
                    Firmware Complete
                           |
                           v
                    Verify CRC
                           |
                    +------+------+
                    |             |
                  Failed        Match
                    |             |
                    v             v
                  Abort       Validate App
                                  |
                                  v
                           Start Application
```

---

## OTA Image Format

The OTA image contains a small header before the actual application firmware.

The header provides the information required by the bootloader to identify and validate the firmware.

```text
OTA Image
+---------------------------+
| Magic                     |
+---------------------------+
| Image Size                |
+---------------------------+
| CRC32                     |
+---------------------------+
| Version                   |
+---------------------------+
|                           |
| Application Firmware      |
|                           |
+---------------------------+
```

The header used by the OTA image is separate from the actual application code. It allows the bootloader to determine the expected firmware size and CRC before and during the update process.

The firmware image is generated before it is provided to the bootloader. The image preparation tools add the required metadata and produce the final OTA image.

---

## OTA Stream

One of the important parts of this implementation is the OTA stream interface.

The OTA update engine does not need to know where the firmware is coming from. It only needs a way to read the next section of the OTA image.

This allows the same OTA update logic to work with different sources.

For example:

```text
                OTA Update Engine
                       |
                 OTA Stream
                       |
        +--------------+--------------+
        |              |              |
      Flash          UART          Ethernet
        |              |              |
      W25Q           SD Card        Network
```

For this demonstration, the OTA image can be stored locally and supplied through the stream interface. The same update engine can later be connected to an actual network or external storage source.

This separation keeps the bootloader OTA logic independent from the communication method.

---

## Application and Bootloader Roles

The application and bootloader have different responsibilities.

The application can handle the higher-level OTA logic, such as checking for a new firmware version, communicating with an update server, and preparing the firmware for the bootloader.

The bootloader is responsible for the lower-level update operations.

It receives or accesses the OTA image, validates the image header, erases the application Flash area, programs the firmware, calculates the CRC, verifies the result, and finally starts the application.

This separation makes it possible to change the communication method without having to redesign the complete OTA update engine.

---

## Key Files

| File                 | What it does                                                |
| -------------------- | ----------------------------------------------------------- |
| `ota_image.h`        | Contains the OTA image used by the demonstration            |
| `app_header.h`       | Defines the application metadata used for validation        |
| `main.c`             | Contains the bootloader startup and OTA update logic        |
| `ota.c / ota.h`      | Handles the OTA image processing and update flow            |
| `STM32xxxx_FLASH.ld` | Defines the Flash layout for the bootloader and application |
| `app_final.py`       | Prepares the OTA image and adds the required metadata       |
| `bin2c.py`           | Converts the OTA image into C data when required            |

> File names can vary depending on the STM32 project and the final implementation.

---

## Flash Memory Layout

The bootloader and application occupy separate regions of Flash.

A typical layout is:

```text
STM32 Flash
+---------------------------+ 0x08000000
|                           |
|       Bootloader          |
|                           |
+---------------------------+
|                           |
|    Application Header     |
|                           |
+---------------------------+
|                           |
|       Application         |
|                           |
|        Firmware           |
|                           |
+---------------------------+
```

The exact addresses depend on the STM32 device and the linker-script configuration.

The bootloader must know the application start address so that it can erase and program the correct Flash region and later jump to the newly installed application.

---

## OTA Firmware Validation

Firmware validation is performed at multiple stages.

First, the OTA image header is checked so that the bootloader can confirm that the received image contains valid metadata.

During programming, the bootloader keeps track of the firmware data and calculates its CRC32.

After the complete image has been written, the calculated CRC is compared with the expected CRC stored in the OTA header.

```text
Received Firmware
       |
       v
Calculate CRC32
       |
       v
Compare With OTA Header CRC
       |
   +---+---+
   |       |
 Failed   Match
   |       |
   v       v
 Abort   Application
         Validation
             |
             v
        Jump to App
```

If the CRC does not match, the update is rejected and the bootloader must not start the corrupted application.

---

## Application Validation

The OTA CRC check confirms that the firmware received by the OTA process matches the expected image.

The existing application validation mechanism is then used to make sure that the application stored in Flash is valid before it is executed.

The validation process uses the application information introduced in the earlier parts of the bootloader series.

This provides an additional layer of protection against starting an incomplete or invalid application.

---

## Project Structure

```text
bootloader/
├── Bootloader/
│   ├── Core/
│   │   ├── Inc/
│   │   │   ├── app_header.h
│   │   │   └── ota.h
│   │   └── Src/
│   │       ├── main.c
│   │       └── ota.c
│   ├── STM32xxxx_FLASH.ld
│   └── .ioc
│
├── Application/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── app_header.h
│   │   └── Src/
│   │       └── main.c
│   ├── STM32xxxx_FLASH.ld
│   └── .ioc
│
└── Tools/
    ├── app_final.py
    └── bin2c.py
```

---

## CubeMX Configuration

The project uses the same basic STM32 configuration as the previous bootloader examples.

| Peripheral | Setting                             |
| ---------- | ----------------------------------- |
| Clock      | MCU-specific configuration          |
| USART      | Optional debug output               |
| GPIO       | User LEDs / status indication       |
| Flash      | Used for bootloader and application |
| NVIC       | Default configuration               |

No specific network peripheral is required for the OTA update engine itself.

The OTA stream can later be connected to a communication interface such as Ethernet, Wi-Fi, UART, USB, SD card, or external Flash.

---

## Creating the OTA Image

The application firmware first needs to be converted into an OTA image.

The image preparation process adds the metadata required by the bootloader.

The resulting image contains:

```text
Metadata Header
       +
Application Binary
```

The metadata includes information such as:

```text
Magic Number
Image Size
CRC32
Version
```

The CRC is calculated from the application firmware and stored in the OTA header.

This allows the bootloader to calculate the CRC again during the update and compare the result with the expected value.

---

## How to Build

1. Clone this repository.
2. Open the Bootloader and Application projects in STM32CubeIDE.
3. Check the Flash layout in the linker scripts.
4. Build the Application project.
5. Generate the application binary required for the OTA image.
6. Run the OTA image preparation script.
7. Generate the final OTA image.
8. Build the Bootloader project.
9. Program the bootloader to the target MCU.
10. Provide the OTA image to the bootloader through the configured OTA stream.
11. Open the UART terminal if debug output is enabled.

> **Important:** Make sure that the application Flash address, image size, linker script, and OTA metadata are consistent with each other.

---

## Testing the OTA Update

Start with a valid application already programmed on the target.

When an OTA update is requested, the bootloader enters the OTA update flow.

It first reads the OTA image header and validates the metadata.

If the image is valid, the application Flash region is erased.

The firmware is then read from the OTA stream and programmed into Flash.

During this process, the bootloader calculates the CRC32 of the firmware.

Once all firmware data has been written, the calculated CRC is compared with the CRC stored in the OTA image.

If the CRC matches, the bootloader continues with application validation and starts the new firmware.

The expected sequence is:

```text
OTA Update Requested
        |
        v
Bootloader Starts OTA Flow
        |
        v
Read OTA Header
        |
        v
Validate Image
        |
        v
Erase Application Flash
        |
        v
Receive Firmware
        |
        v
Program Flash + Calculate CRC
        |
        v
Firmware Complete
        |
        v
Verify CRC
        |
        v
Validate Application
        |
        v
Jump to New Application
```

---

## What Happens If the CRC Fails?

If the calculated CRC does not match the CRC stored in the OTA image header, the bootloader treats the update as invalid.

The new firmware must not be started because the image may be incomplete or corrupted.

The bootloader should remain in its update or error path rather than jumping to an invalid application.

This CRC check is therefore an important part of the OTA update process.

---

## Why Use an OTA Stream?

The OTA stream separates **how the firmware is received** from **how the firmware is installed**.

The bootloader update engine only needs to request a block of data from the stream.

The source of that data can be changed independently.

For example:

```text
UART
  |
  +----+
       |
Ethernet ---> OTA Stream ---> OTA Update Engine ---> STM32 Flash
       |
  +----+
  |
W25Q / SD Card / Other Source
```

This makes the OTA update engine reusable across different STM32 projects.

A future implementation can replace the demonstration stream with a real network-based firmware download without changing the core Flash programming and validation logic.

---

## Series

| Part       | Topic                                       | Link                                                                          |
| ---------- | ------------------------------------------- | ----------------------------------------------------------------------------- |
| Part 1     | Flash Layout & Jump to Application          | https://controllerstech.com/stm32-custom-bootloader-tutorial/                 |
| Part 2     | Application Validation Using Magic Number   | https://controllerstech.com/stm32-bootloader-application-validation/          |
| Part 3     | Application Size & CRC Validation           | https://controllerstech.com/stm32-bootloader-application-crc-size-validation/ |
| Part 4     | OTA Bootloader FLAG Mechanism               | https://controllerstech.com/stm32-ota-bootloader-flag-mechanism/              |
| **Part 5** | **Full OTA Firmware Update (this project)** | https://controllerstech.com/stm32-ota-bootloader-full-update/                 |

---

## What's Next?

This tutorial completes the basic OTA firmware update engine.

The next step is to connect the OTA stream to a real firmware source. The same update mechanism can then be used with communication interfaces such as Ethernet, Wi-Fi, cellular, UART, USB, or external Flash.

The important part is that the bootloader's core update process does not need to change. Only the source used by the OTA stream needs to provide the firmware data.

---

## License

Open source — free to use and modify.

If this project helped you, consider supporting the work:

https://paypal.me/controllertech
