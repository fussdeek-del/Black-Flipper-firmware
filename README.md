# Black-Flipper-firmware

ESP-IDF v5.x firmware skeleton for the **Flipper Black** — a custom multi-radio handheld security/research device built on the ESP32-S3.

Hardware reference: [Black-Flipper](https://github.com/fussdeek-del/Black-Flipper)

Architecture details: [docs/DESIGN.md](docs/DESIGN.md)

## Features (skeleton stage)

- Dual-core FreeRTOS task layout (Core 0: RF/IR, Core 1: UI/system/GPS)
- Driver placeholders for CC1101, SX1262, PN532, MAX-M8Q, ST7796S, IR, W25Q128, battery
- Inter-core event bus (FreeRTOS queue)
- UI menu skeleton with LVGL integration notes (logging-only for now)

## Project tree

```text
Black-Flipper-firmware/
├── CMakeLists.txt
├── sdkconfig.defaults
├── main/
│   ├── CMakeLists.txt
│   └── main.c
├── components/
│   ├── config/
│   │   ├── CMakeLists.txt
│   │   └── include/app_config.h
│   ├── drivers/
│   │   ├── CMakeLists.txt
│   │   ├── include/          # cc1101, sx1262, pn532, max_m8q, st7796s, ir, w25q128, battery
│   │   └── *.c
│   ├── system/
│   │   ├── CMakeLists.txt
│   │   ├── include/          # task_manager.h, event_bus.h
│   │   ├── task_manager.c
│   │   └── event_bus.c
│   └── ui/
│       ├── CMakeLists.txt
│       ├── include/          # ui_manager.h, screens.h
│       └── ui_manager.c
└── docs/
    └── DESIGN.md
```

## Prerequisites

- [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/) installed and `IDF_PATH` set
- ESP32-S3 target (Flipper Black MCU)

## Build

```bash
cd Black-Flipper-firmware
idf.py set-target esp32s3
idf.py build
```

## Flash

Replace `PORT` with your serial port (e.g. `COM3` on Windows, `/dev/ttyUSB0` on Linux):

```bash
idf.py -p PORT flash monitor
```

## Status

This repository is a **firmware skeleton** — placeholder drivers and simulated UI events only. GPIO pinout in `components/config/include/app_config.h` is provisional until verified against the KiCad schematic.
