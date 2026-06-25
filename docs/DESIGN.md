# Flipper Black: Firmware Design & System Architecture

This document describes the firmware architecture, core scheduling allocation, peripheral pinouts, and UI design flow for the **Flipper Black** — a custom multi-radio security and research handheld device powered by the ESP32-S3 (QFN56 package).

---

## 1. High-Level Block Diagram

The Flipper Black firmware uses a dual-core architecture where real-time communications are isolated from system tasks and UI rendering.

```
       +--------------------------------------------------------+
       |                  ESP32-S3 (QFN56 MCU)                  |
       +----------------------------+---------------------------+
       |     Core 0 (PRO CPU)       |     Core 1 (APP CPU)      |
       |  Time-Critical RF Engine   |  System Services & UI     |
       +----------------------------+---------------------------+
       |                            |                           |
       |  +----------------------+  |  +---------------------+  |
       |  |     RF Core Task     |  |  |    UI Core Task     |  |
       |  | (CC1101 / SX1262 RX) |  |  | (ST7796S / Touch)   |  |
       |  +-----------+----------+  |  +----------+----------+  |
       |              |             |             |             |
       |  +-----------v----------+  |  +----------v----------+  |
       |  |     IR Core Task     |  |  |   System Core Task  |  |
       |  |      (RMT Tx/Rx)     |  |  |   (PN532 / Battery) |  |
       |  +----------------------+  |  +----------+----------+  |
       |                            |             |             |
       |                            |  +----------v----------+  |
       |                            |  |    GPS Core Task    |  |
       |                            |  |      (MAX-M8Q)      |  |
       |                            |  +---------------------+  |
       +----------------------------+---------------------------+
                                   ^
                                   | (Event Bus Queue)
                                   v
       +--------------------------------------------------------+
       |         FreeRTOS Queue-Based Thread-Safe Bus           |
       +--------------------------------------------------------+
```

---

## 2. Core Allocation & Thread Model

### Core 0 (PRO CPU) - RF Core
Dedicated entirely to high-frequency and time-sensitive signal actions. By removing all UI drawing logic and slow serial polling from this core, we ensure zero packet loss during rapid transmission or reception phases.
* **`RF_Core_Task` (Priority 10)**: Polls the status of the CC1101 and SX1262 SPI lines or handles hardware interrupts. Decodes raw packets on-the-fly and drops them into the Event Bus.
* **`IR_Core_Task` (Priority 9)**: Manages RMT (Remote Control) carrier wave synthesis for transmitting IR signals and registers incoming infrared pulse periods.

### Core 1 (APP CPU) - System & UI Core
Handles user experience, human-device interfaces (HID), slower sensing lines, and location logging.
* **`UI_Core_Task` (Priority 4)**: Runs the graphics loop (LVGL), reads touch coordinates, renders UI components to the ST7796S LCD, and processes screen transitions.
* **`System_Core_Task` (Priority 6)**: Polls the TP4056 charging status pins, reads the battery ADC, and commands the PN532 NFC reader over the shared I2C bus.
* **`GPS_Core_Task` (Priority 3)**: Monitors the UART ring buffer for serial data arriving from the MAX-M8Q GPS receiver, parses standard NMEA streams, and registers latitude/longitude coordinates.

---

## 3. Peripheral Pinout Table (ESP32-S3 QFN56)

| Peripheral Module | Pin Function | GPIO Pin | Shared Bus | Electrical Config / Notes |
|:---|:---|:---|:---|:---|
| **RF SPI** | SCLK | GPIO 12 | SPI2 | 40MHz clock line |
| | MISO | GPIO 13 | SPI2 | Input with pull-up |
| | MOSI | GPIO 14 | SPI2 | Output |
| **CC1101** | CS | GPIO 11 | SPI2 | Active-Low chip select |
| | GDO0 | GPIO 10 | — | Rising/falling edge interrupt input |
| | GDO2 | GPIO 9 | — | Digital input (carrier sense check) |
| **SX1262** | CS | GPIO 15 | SPI2 | Active-Low chip select |
| | DIO1 | GPIO 16 | — | Packet transaction done interrupt input |
| | BUSY | GPIO 17 | — | Active-High state check input |
| | RST | GPIO 18 | — | Hardware reset output line |
| **Display SPI** | SCLK | GPIO 37 | SPI3 | 40MHz clock line |
| | MOSI | GPIO 35 | SPI3 | Output |
| | MISO | GPIO 36 | SPI3 | Input |
| **ST7796S LCD** | CS | GPIO 38 | SPI3 | Active-Low screen chip select |
| | DC | GPIO 39 | — | Command (Low) / Data (High) output |
| | RST | GPIO 40 | — | Active-Low hardware screen reset |
| | BCKL | GPIO 41 | — | PWM backlight drive (LEDC Channel) |
| **Touch Sensor** | CS | GPIO 42 | SPI3 | Active-Low touch chip select |
| | IRQ | GPIO 2 | — | Interrupt input on touch touch-down |
| **W25Q128 Flash**| CS | GPIO 47 | SPI3 | Active-Low external SPI flash CS |
| **PN532 NFC** | SDA | GPIO 4 | I2C0 | I2C clock (4.7k external pull-up) |
| | SCL | GPIO 5 | I2C0 | I2C data (4.7k external pull-up) |
| | IRQ | GPIO 6 | — | NFC tag found wake interrupt |
| | RST | GPIO 7 | — | Hardware sleep reset output |
| **MAX-M8Q GPS**  | TX | GPIO 43 | UART2 | Connects to GPS RX (9600 baud) |
| | RX | GPIO 44 | UART2 | Connects to GPS TX (9600 baud) |
| **Infrared** | TX | GPIO 45 | RMT | Output to IR LED driver (38kHz carrier) |
| | RX | GPIO 46 | RMT | Input from IR receiver block |
| **Power (TP4056)**| ADC | GPIO 1 | ADC1_CH0 | Analog voltage sense (1/2 resistor divider)|
| | CHG | GPIO 3 | — | Charging state (Active-Low open-drain) |
| | STBY | GPIO 8 | — | Standby/Charged state (Active-Low) |
| **USB Native** | D- | GPIO 19 | — | Built-in ESP32-S3 USB OTG |
| | D+ | GPIO 20 | — | Built-in ESP32-S3 USB OTG |
| **Boot Pin** | BOOT | GPIO 0 | — | Internally pulled high, ground to program |

---

## 4. Software Layering Architecture

The firmware separates physical hardware commands from system tasks and UI logic:

```
+------------------------------------------------------------+
|                      User Interface                        |
|   (screens.h, ui_manager.c: menus, graphics, lists)        |
+------------------------------+-----------------------------+
                               | Event callbacks & states
+------------------------------v-----------------------------+
|                     Services & Logic                       |
|   (task_manager.c, event_bus.c: event dispatchers)         |
+------------------------------+-----------------------------+
                               | Driver function calls
+------------------------------v-----------------------------+
|                 Hardware Abstraction Layer                 |
|   (cc1101.c, sx1262.c, pn532.c, battery.c, st7796s.c...)    |
+------------------------------+-----------------------------+
                               | ESP-IDF drivers API
+------------------------------v-----------------------------+
|                   ESP-IDF System Layer                     |
|   (driver/spi_master, driver/i2c, esp_adc, driver/rmt...)  |
+------------------------------------------------------------+
```

1. **ESP-IDF System Layer**: Provides core libraries for peripheral bus transactions (I2C, SPI, UART, ADC, RMT).
2. **Hardware Abstraction Layer (HAL / Drivers)**: Implements register configs, chip resets, state machines, and raw read/write APIs for each unique peripheral chip.
3. **Services & Logic Layer**: Orchestrates execution timing using FreeRTOS tasks. Emits system-wide status signals through the centralized `event_bus`.
4. **User Interface Layer**: Consumes the event queue. Dynamically redraws widgets, sets screen backlights, and presents current status parameters.

---

## 5. UI Screen Flow

The user interface uses a cyclic layout style suitable for touchscreen and button inputs:

```
               +-----------------------+
               |       Main Menu       |
               +-----------+-----------+
                           | Touch press
                           v
               +-----------------------+
               |     Sub-GHz Tools     |  <-- CC1101 status / RX list
               +-----------+-----------+
                           | Touch press
                           v
               +-----------------------+
               |      LoRa Tools       |  <-- SX1262 details / RSSI meter
               +-----------+-----------+
                           | Touch press
                           v
               +-----------------------+
               |      NFC Scanner      |  <-- PN532 card reader card UID
               +-----------+-----------+
                           | Touch press
                           v
               +-----------------------+
               |    GPS Coordinates    |  <-- Latitude / Longitude
               +-----------+-----------+
                           | Touch press
                           v
               +-----------------------+
               |    System Settings    |  <-- Backlight, power, flash storage
               +-----------+-----------+
                           | Touch press
                           v
                       (Back to Main Menu)
```

- **Status Bar**: Standard bar drawn at the top of every screen containing the battery capacity indicator (`Bat: X%`), charging status icon, active GPS lock indicator (`Fix` / `NoFix`), and local clock display.
- **Navigation**: Swiping or tapping anywhere on the screen cycles to the next functional utility panel.

---

## 6. Future Expansion Roadmap

1. **Direct Memory Access (DMA) SPI Channels**: Configure SPI3 to use DMA buffers when flushing pixel colors to the ST7796S display, freeing CPU cycles during UI updates.
2. **FatFS/SPIFFS File Explorer Screen**: Develop screen flows to traverse files stored in the external W25Q128 memory partition, allowing users to save captured sub-GHz signals or GPS track logs.
3. **Signal Analyzer Module**: Use CC1101 interrupt lines to measure exact sub-GHz RF pulse timings (amplitudes/intervals) and save them as `.sub` raw capture files.
4. **Low Power Sleep Modes**: Put Core 0 tasks to sleep and command the GPS/NFC/RF chips into low power standbys when the device is idle, waking only on touch interrupts or battery charge events.
