# STM32 Custom Bootloader — OTA Update FLAG Mechanism

Part 4 of the STM32 Custom Bootloader series. In the previous parts, the bootloader was extended with application validation using a magic number, application size, reset handler, and CRC32. This tutorial adds a simple mechanism that allows the application to request an OTA update from the bootloader. The application sets an OTA flag in Flash and then resets the MCU. After the reset, the bootloader reads this flag and decides whether it should start the OTA update process or continue with the normal application validation and jump.

## 📺 Video Tutorial

[STM32 OTA Bootloader: FLAG Mechanism](https://youtu.be/t7Lcx4RigB0)

## 📖 Full Article

[STM32 OTA Bootloader: FLAG Mechanism](https://controllerstech.com/stm32-ota-bootloader-flag-mechanism/)

---

## Hardware Used

| Component | Details                                                                                            |
| --------- | -------------------------------------------------------------------------------------------------- |
| Board     | STM32 Nucleo L496ZG *(any STM32 with sufficient Flash can be used by adjusting the memory layout)* |
| UART      | ST-Link Virtual COM Port                                                                           |
| LEDs      | Onboard User LEDs                                                                                  |
| Button    | GPIO input used to simulate an OTA request                                                         |

### Pin Assignment

| Signal     | Pin         | Function                             |
| ---------- | ----------- | ------------------------------------ |
| UART TX    | ST-Link VCP | Bootloader debug messages            |
| UART RX    | ST-Link VCP | Optional                             |
| User LED   | Onboard     | Indicates the application is running |
| Error LED  | Onboard     | Indicates validation or OTA failure  |
| OTA Button | PA8         | Simulates an OTA update request      |

> No network connection is required for this tutorial. The button is used to simulate an OTA request. The actual firmware download and network communication will be added in later tutorials.

---

## What This Project Does

* Extends the custom bootloader with an OTA request mechanism
* Uses the existing application header to store an OTA flag
* Allows the application to request an OTA update
* Stores the OTA request in Flash so it survives an MCU reset
* Resets the MCU after setting the OTA flag
* Checks the OTA flag when the bootloader starts
* Clears the OTA flag after detecting the request
* Enters the OTA update flow when the flag is set
* Continues with normal application validation when the flag is not set
* Keeps the application and bootloader responsibilities separate
* Uses a button to simulate the OTA request in this tutorial

---

## How It Works

In a real product, the application is normally responsible for handling the communication with the update server. It can connect to the server, check whether a new firmware version is available, download the new firmware, and decide when the system is ready for the update.

The bootloader should remain simple and stable. Its main job is to handle the firmware update, validate the new application, program the Flash, and start the application. It does not need to handle the complete network communication used to decide when an update is available.

Once the application is ready to start an update, it sets an OTA flag in the application header stored in Flash. The application then resets the MCU.

When the MCU resets, the bootloader starts first. It reads the application header and checks the OTA flag. If the flag is set, the bootloader knows that the application has requested an OTA update. It clears the flag and enters the OTA update flow.

If the OTA flag is not set, the bootloader continues with its normal startup process. It validates the application using the existing checks and, if the application is valid, jumps to it.

The basic flow is:

```text
Application
     |
     | OTA update requested
     v
Set OTA Flag in Flash
     |
     v
Reset MCU
     |
     v
Bootloader Starts
     |
     v
Read OTA Flag
     |
     +----------------------+
     |                      |
   Flag = 1              Flag = 0
     |                      |
     v                      v
Clear Flag          Validate Application
     |                      |
     v                      v
OTA Update          Jump to Application
```

This provides a simple way for the application to communicate with the bootloader across a reset.

### Key Files

| File                               | What it does                                                                                         |
| ---------------------------------- | ---------------------------------------------------------------------------------------------------- |
| `app_header.h`                     | Defines the shared application header and OTA flag                                                   |
| `main.c` (Bootloader)              | Reads the OTA flag, handles the OTA request, validates the application, and jumps to the application |
| `main.c` (Application)             | Detects the OTA request and sets the OTA flag before resetting the MCU                               |
| `STM32xxxx_FLASH.ld` (Application) | Places the application header at the fixed Flash address                                             |
| `system_stm32xxxx.c`               | Relocates the application's vector table                                                             |

---

## OTA Flag

The OTA flag is stored inside the application header in Flash. A value of `1` indicates that an OTA update has been requested.

```text
Application Header
+---------------------------+
| OTA Flag                  |
+---------------------------+
| Magic Number              |
+---------------------------+
| Application Size          |
+---------------------------+
| Application CRC32         |
+---------------------------+
| Firmware Version / ...    |
+---------------------------+
```

When the application wants to start an OTA update, it changes the OTA flag to `1` and resets the MCU.

After the reset, the bootloader reads the flag. If the value is `1`, it clears the flag and starts the OTA update process.

Clearing the flag is important because the MCU may reset again during the update. The same OTA request should not be processed repeatedly after every reset.

---

## Application and Bootloader Roles

The application and bootloader have different responsibilities.

The application handles the higher-level update logic. In a real product, this can include connecting to a server, checking the firmware version, downloading the firmware, and checking that the downloaded data is ready.

The bootloader handles the lower-level firmware operations. It can receive or access the firmware image, erase and program Flash, validate the new application, and finally start it.

This separation keeps the bootloader small and stable while allowing the application to support different communication methods such as Ethernet, Wi-Fi, cellular, or another connection.

---

## Project Structure

```text
bootloader/
├── Bootloader/
│   ├── Core/
│   │   ├── Inc/
│   │   │   └── app_header.h
│   │   └── Src/
│   │       └── main.c              ← OTA flag and bootloader logic
│   ├── STM32xxxx_FLASH.ld
│   └── .ioc
│
└── Application/
    ├── Core/
    │   ├── Inc/
    │   │   └── app_header.h
    │   └── Src/
    │       └── main.c              ← OTA request and flag handling
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

The application header is stored at a known Flash address so both the application and bootloader can access the OTA flag.

The header also contains the information used by the previous application validation tutorials, including the magic number, application size, and CRC32.

---

## CubeMX Configuration

| Peripheral | Setting                                |
| ---------- | -------------------------------------- |
| Clock      | Same configuration as previous parts   |
| USART      | Enabled for bootloader debug output    |
| GPIO       | User LEDs                              |
| GPIO PA8   | Input used to simulate the OTA request |
| NVIC       | Default configuration                  |

This project builds on the previous bootloader projects. No network peripheral is required for this tutorial.

The button connected to **PA8** is used to simulate an OTA request. In a real product, this request would normally come from the application after it has detected and prepared a firmware update.

---

## How to Build

1. Clone this repository.
2. Open both the Bootloader and Application projects in STM32CubeIDE.
3. Generate code if required.
4. Build the Application project.
5. Program the Application **ELF** file to the target.
6. Build the Bootloader project.
7. Program the Bootloader at `0x08000000`.
8. Reset the board.
9. Open the UART terminal to view the bootloader messages.

> **Important:** Flash the **ELF** (or HEX) file for the application rather than the BIN file. The application header is stored in a dedicated linker section, and ELF/HEX files preserve the section addresses correctly.

---

## Testing

With a valid application programmed, resetting the board produces the normal bootloader sequence.

The bootloader starts, reads the application header, and checks the OTA flag. Since the flag is normally cleared, the bootloader continues with the normal application validation process.

If the application passes the validation checks, the bootloader jumps to the application and the application starts normally.

### Testing the OTA Request

To simulate an OTA request, press the button connected to **PA8** while the application is running.

The application detects the button press and sets the OTA flag in Flash. It then resets the MCU.

After the reset, the bootloader starts and reads the OTA flag. Since the flag is set, the bootloader recognizes that an OTA update has been requested.

The bootloader clears the flag and enters the OTA update flow.

For this tutorial, the actual firmware download is not implemented yet. The OTA process is simulated using the bootloader messages and LED indication.

The expected flow is:

```text
Application Running
       |
       v
PA8 Button Pressed
       |
       v
Set OTA Flag = 1
       |
       v
Reset MCU
       |
       v
Bootloader Starts
       |
       v
OTA Flag Detected
       |
       v
Clear OTA Flag
       |
       v
Enter OTA Update Flow
```

### Testing a Normal Boot

After the OTA flag has been cleared, reset the MCU again without requesting an OTA update.

The bootloader should detect that the OTA flag is not set. It then performs the normal application validation and jumps to the application.

This confirms that the OTA request is handled only when the flag is present.

---

## Why the OTA Flag Is Needed

The application and bootloader run at different stages of the startup process. When the application is running, the bootloader is no longer active.

The application therefore needs a simple way to tell the bootloader what should happen after the next reset.

The OTA flag provides this communication method. Since the flag is stored in Flash, it remains available after the MCU resets.

The application only needs to set the flag and reset the MCU. The bootloader then reads the flag during startup and chooses the correct path.

This approach is simple and does not require the application to directly call or control the bootloader code.

---

## Series

| Part       | Topic                                            | Link                                                                          |
| ---------- | ------------------------------------------------ | ----------------------------------------------------------------------------- |
| Part 1     | Flash Layout & Jump to Application               | https://controllerstech.com/stm32-custom-bootloader-tutorial/                 |
| Part 2     | Application Validation Using Magic Number        | https://controllerstech.com/stm32-bootloader-application-validation/          |
| Part 3     | Application Size & CRC Validation                | https://controllerstech.com/stm32-bootloader-application-crc-size-validation/ |
| **Part 4** | **OTA Bootloader FLAG Mechanism (this project)** | https://controllerstech.com/stm32-ota-bootloader-flag-mechanism/              |

---

## Next Part

In the next tutorial, we will move from the OTA request mechanism to the actual firmware update process. The bootloader will use the OTA request to enter an update mode, receive the new firmware, erase the required Flash area, and program the new application.

The updated application will then be checked using the existing validation mechanism, including the application size and CRC32. This allows the bootloader to make sure that the new firmware was written correctly before starting it.

**Part 5:** STM32 Custom Bootloader: OTA Firmware Update

---

## License

Open source — free to use and modify. If this project helped you, consider supporting the work:

https://paypal.me/controllertech
