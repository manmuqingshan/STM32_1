# STM32 USB Host Audio Class — WAV Player from SD Card

Part 10 of the STM32 USB series. This project configures an STM32 Nucleo L496 as a USB Host using the Audio Class. It reads WAV files from an SD card over SPI and streams the audio directly to a USB sound card or headphones, no external codec required. Playback is controlled using three buttons for Next, Previous, and Play/Pause.

---

## 📺 Video Tutorial

[STM32 USB Host Audio Class: Play WAV Files from SD Card — Video Tutorial | USB Series #10](https://youtu.be/tMGZ4x-mDeQ)

## 📖 Full Article

[STM32 USB Host Audio Class: Play WAV Files from an SD Card](https://controllerstech.com/stm32-usb-host-audio-class-sd-card/)

---

## Hardware Used

| Component | Details |
|-----------|---------|
| Board | STM32 Nucleo L496ZG |
| Storage | SD card module (SPI, with built-in 5V to 3.3V voltage shifter) |
| Audio Output | USB sound card + speaker / headphones |
| Interface | USB OTG FS — Host Only mode |
| UART | LPUART1 — ST-Link virtual COM port |

### Pin Assignment

| Signal | Pin | Function |
|--------|-----|----------|
| SPI1 SCK | PA5 | SD card clock |
| SPI1 MISO | PA6 | SD card data in |
| SPI1 MOSI | PA7 | SD card data out |
| SD_CS | PD14 | SD card chip select, GPIO Output, default High |
| Next Button | PA3 | EXTI, Falling Edge, Pull-Up |
| Previous Button | PC0 | EXTI, Falling Edge, Pull-Up |
| Play/Pause Button | PC1 | EXTI, Falling Edge, Pull-Up |
| USB D− | PA11 | USB OTG FS (auto-assigned) |
| USB D+ | PA12 | USB OTG FS (auto-assigned) |
| VBUS Sense | PA9 | USB VBUS sensing (auto-assigned) |
| USB Power Switch Enable | PC6 | GPIO Output, default High |
| LPUART1 TX | PG7 | ST-Link virtual COM port TX |
| LPUART1 RX | PG8 | ST-Link virtual COM port RX |

> PC6 must be set high to enable the USB power switch IC on the Nucleo L496. Without this, the connected USB sound card will not receive power and will not be detected.

---

## What This Project Does

- Configures USB OTG FS in Host Only mode using STM32CubeMX
- Uses the USB Audio Host class to stream PCM audio to a USB sound card or headphones
- Configures SPI1 with DMA to read and write the SD card at a usable data rate
- Implements a custom FatFs disk I/O driver (`sd_spi.c` + `sd_diskio_spi.c`) instead of the default CubeMX-generated one
- Scans the SD card for `.wav` files and builds a playlist (`explorer.c`)
- Reads the WAV header, fills a ring buffer, and streams audio through `AUDIO_Start()` and `AUDIO_Process()` (`audio.c`)
- Handles Next, Previous, and Play/Pause using external interrupts, with debounce handling
- Loops back to the first track automatically once the last track finishes

---

## How It Works

The USB host stack enumerates the connected sound card automatically. Once the device is detected, the `USBH_UserProcess` callback fires with the `HOST_USER_CLASS_ACTIVE` state, after which `USB_AppProcess()` checks `Appli_state` and `hUsbHostFS.gState` before initializing the SD card and starting playback.

The SD card is mounted using a custom FatFs driver, since the class is set to **User-Defined** mode in CubeMX. `SD_StorageParse()` walks the file system, adds every `.wav` file to `FileList`, and this list is used later to open tracks by index.

`AUDIO_Start()` opens a track by index, reads its WAV header into `WavInfo`, sets the sample rate on the USB device with `USBH_AUDIO_SetFrequency()`, and fills the ring buffer for the first time. `AUDIO_Process()` runs continuously in the main loop and keeps the buffer topped up as the USB device consumes it, based on the offset returned by `USBH_AUDIO_GetOutOffset()`.

### Button Debounce

Button presses are captured in `HAL_GPIO_EXTI_Callback()` and stored in `key_pressed`. The main loop adds a 250 ms delay before processing the key, since without it, a single press was seen to register as multiple presses and skip several tracks at once.

### Playback State Machine

Playback runs on a state machine in `audio.c`, with states for config, play, next, previous, pause, resume, and error. `AUDIO_PlaybackKeys()` maps a button press to a state change, and `AUDIO_Process()` handles the actual behavior for each state on every loop iteration.

> The player loops back to track 0 once it passes the last track in the `AUDIO_STATE_NEXT` case. Replace this wrap-around with a call to `AUDIO_Stop()` if you want playback to stop at the end instead.

### Key Files

| File | What it does |
|------|--------------|
| `main.c` | Added `_write()` for printf routing, `USB_AppProcess()` for SD init and playback start, `HAL_GPIO_EXTI_Callback()` for button capture |
| `explorer.c` | Mounts the SD card, links the custom FatFs driver, scans for WAV files and builds `FileList` |
| `audio.c` | `AUDIO_Start()`, `AUDIO_Process()`, `AUDIO_Stop()`, `AUDIO_PlaybackKeys()` — the full playback state machine |
| `sd_spi.c` | Low-level SPI communication with the SD card — commands, block read/write, card init |
| `sd_diskio_spi.c` | Connects `sd_spi.c` to FatFs (`disk_initialize`, `disk_read`, `disk_write`, `disk_ioctl`) |

---

## CubeMX Configuration Summary

| Peripheral | Setting |
|------------|---------|
| USB OTG FS | Host Only mode |
| USB Host Class | Audio Host Class |
| VBUS Sensing | Enabled — PA9 |
| SPI1 | Full-Duplex Master, 8-bit, MSB first, ~5 Mbps, DMA enabled (TX + RX, Normal, Byte) |
| FatFs | User-Defined mode (custom disk I/O driver) |
| Buttons | PA3, PC0, PC1 — EXTI, Falling Edge, Pull-Up |
| PC6 | GPIO Output, default High (USB power switch enable) |
| LPUART1 | 115200 baud, 8N1, TX → PG7, RX → PG8 |

### Clock

| Setting | Value |
|---------|-------|
| HSI | 16 MHz internal oscillator |
| System Clock | 80 MHz (via PLL) |
| USB Clock | 48 MHz (HSI48 dedicated oscillator) |

---

## Testing

Flash the project and open a serial monitor at **115200 baud, 8N1** on the ST-Link virtual COM port.

Copy 44.1 kHz stereo WAV files onto a FAT32-formatted SD card, insert it, and connect a USB sound card with a speaker or headphones attached. On reset, the console will print:
```
Device Connected
Device Ready
SD card mounted successfully at ...
Total WAV files found : 5
Playing file (1/5): TRACK1.WAV
```

Elapsed time prints every second while a track plays:
```
[00:01]
[00:02]
[00:03]
```

**Playback control:** Press the Next or Previous button to skip tracks, and the console prints the newly selected file. Press Play/Pause to suspend and resume the audio stream. The player loops back to track 1 automatically after the last track finishes.

---

## Series

| Part | Topic | Link |
|------|-------|------|
| Part 1 | CDC — Virtual COM Port | [Article](https://controllerstech.com/stm32-usb-cdc-virtual-com-port/) |
| Part 2 | HID — Gamepad / Joystick | [Article](https://controllerstech.com/stm32-usb-hid-gamepad/) |
| Part 3 | HID — Mouse + Keyboard (separate projects) | [Article](https://controllerstech.com/stm32-usb-hid-mouse-keyboard/) |
| Part 4 | MSC — SD Card as External Drive | [Article](https://controllerstech.com/stm32-usb-mass-storage-class-sd-card/) |
| Part 5 | MSC — W25Q NOR Flash as External Drive | [Article](https://controllerstech.com/stm32-usb-mass-storage-class-w25q-nor-flash/) |
| Part 6 | Composite Class — CDC + HID Gamepad | [Article](https://controllerstech.com/stm32-usb-composite-class-cdc-hid/) |
| Part 7 | HID — Mouse + Keyboard Combined with Report IDs | [Article](https://controllerstech.com/stm32-usb-hid-mouse-and-keyboard-combined/) |
| Part 8 | USB Host MSC — Read & Write USB Flash Drive | [Article](https://controllerstech.com/stm32-usb-host-mass-storage-fatfs/) |
| Part 9 | USB Host HID — Keyboard & Mouse | [Article](https://controllerstech.com/stm32-usb-host-hid-keyboard-mouse/) |
| **Part 10** | **USB Host Audio Class — WAV Player from SD Card (this project)** | [Article](https://controllerstech.com/stm32-usb-host-audio-class-sd-card/) |

---

## License

Open source — free to use and modify. If this project helped you, consider [supporting the work](https://paypal.me/controllertech).
