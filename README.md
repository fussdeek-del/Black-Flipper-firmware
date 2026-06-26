# Flipper Black Firmware

ESP-IDF v5.x firmware skeleton for the **Flipper Black** — a custom open-source multi-radio handheld built on the ESP32-S3. This repo provides the project structure, driver stubs, dual-core task layout, and architecture docs. It is a starting point for firmware development, not a finished product.

**Hardware repo:** [Black-Flipper](https://github.com/fussdeek-del/Black-Flipper)  
**Architecture doc:** [docs/DESIGN.md](docs/DESIGN.md)

---

## Hardware overview

| Subsystem | Chip | Interface |
|-----------|------|-----------|
| MCU | ESP32-S3 (QFN56) | Dual-core @ 240 MHz, Wi-Fi + BLE |
| Sub-GHz | CC1101 + PA | SPI (shared RF bus) |
| LoRa | SX1262 | SPI (shared RF bus) |
| NFC | PN532 | I2C |
| GPS | MAX-M8Q | UART |
| Display | ST7796S 3.5" IPS | SPI |
| Touch | FT6336U | I2C |
| Flash | W25Q128 (16 MB) | SPI (shared display bus) |
| IR | TX + RX | RMT / GPIO |
| Power | TP4056 + DW01A | ADC + status GPIO |

GPIO assignments live in [`components/config/include/app_config.h`](components/config/include/app_config.h). They are **provisional placeholders** until cross-checked against the KiCad schematic in the hardware repo.

---

## Firmware status

| Area | Status |
|------|--------|
| Project structure & CMake | Done |
| Dual-core FreeRTOS tasks | Skeleton |
| Inter-core event bus | Skeleton |
| Peripheral drivers | Placeholder stubs (`// TODO`) |
| UI / LVGL | Logging-only menu skeleton |
| Wi-Fi / BLE / MicroSD / MCP23017 | Not started |

There is no real register-level driver logic yet. Functions log intent and return `ESP_OK` where appropriate so the layout can be filled in incrementally.

---

## Architecture

The firmware splits work across the two ESP32-S3 cores:

```
Core 0 (PRO CPU) — real-time RF
├── RF_Core_Task     CC1101 + SX1262 polling / interrupts
└── IR_Core_Task     Infrared TX/RX (RMT)

Core 1 (APP CPU) — system & UI
├── System_Core_Task PN532, battery, charging status
├── GPS_Core_Task    MAX-M8Q NMEA parsing
└── UI_Core_Task     Display, touch, menu (LVGL planned)

         ┌─────────────────┐
         │   Event Bus     │  FreeRTOS queue (app_event_t)
         └─────────────────┘
              ↑ post          ↓ receive
         Core 0 / 1 tasks     UI task
```

**Software layers:** ESP-IDF HAL → `drv_*` drivers → `task_manager` / `event_bus` → `ui_manager` / `screens`

See [docs/DESIGN.md](docs/DESIGN.md) for block diagrams, pinout tables, UI flow, and roadmap.

---

## Project structure

```text
Black-Flipper-firmware/
├── CMakeLists.txt              # Root ESP-IDF project
├── sdkconfig.defaults          # ESP32-S3, 16 MB flash defaults
├── main/
│   ├── CMakeLists.txt
│   └── main.c                  # app_main(), NVS init, task spawn
├── components/
│   ├── config/
│   │   └── include/app_config.h    # Pins, task config, event types
│   ├── drivers/
│   │   ├── include/                # Driver headers (cc1101, sx1262, …)
│   │   ├── cc1101.c                # Sub-GHz
│   │   ├── sx1262.c                # LoRa
│   │   ├── pn532.c                 # NFC
│   │   ├── max_m8q.c               # GPS
│   │   ├── st7796s.c               # Display
│   │   ├── ir.c                    # Infrared
│   │   ├── w25q128.c               # External flash
│   │   └── battery.c               # TP4056 + ADC
│   ├── system/
│   │   ├── task_manager.c          # Core 0 / Core 1 task loops
│   │   └── event_bus.c             # Inter-core message queue
│   └── ui/
│       ├── ui_manager.c            # Menu + status bar skeleton
│       └── include/screens.h       # Screen IDs
└── docs/
    └── DESIGN.md
```

### Naming conventions

| Prefix | Usage |
|--------|--------|
| `drv_` | Hardware driver functions |
| `app_` | Global config and event types |
| `ui_` | User interface |
| `rf_` / task names | Core 0 radio tasks |

---

## UI menu flow (planned)

The skeleton cycles through these screens on simulated touch events:

```
Main Menu → Sub-GHz Tools → LoRa Tools → NFC Scanner
         → GPS Coordinates → IR Tools → Settings → (wrap)
```

Status bar (planned): battery %, charging icon, GPS fix, clock.

LVGL integration is documented in comments inside `ui_manager.c` but not wired up yet.

---

## Getting started

### Prerequisites

- [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/) with environment exported (`IDF_PATH` set)
- Flipper Black hardware (or ESP32-S3 dev board for early bring-up)

### Build & flash

```bash
cd Black-Flipper-firmware
idf.py set-target esp32s3
idf.py build
idf.py -p PORT flash monitor
```

Replace `PORT` with your serial device (`COM3` on Windows, `/dev/ttyUSB0` on Linux).

---

## Configuration

All board-level settings are centralized in one header:

- **GPIO pinout** — SPI, I2C, UART, RMT, ADC
- **FreeRTOS** — core affinity, priorities, stack sizes
- **Events** — `app_event_type_t` and `app_event_t` payload union

Edit [`components/config/include/app_config.h`](components/config/include/app_config.h) when schematic-verified pins are available.

---

## Roadmap

1. Verify GPIO map against KiCad netlist
2. Implement SPI buses (RF + display) and one driver at a time
3. Add LVGL + FT6336U touch input
4. Wire event bus to real hardware interrupts (GDO0, DIO1, PN532 IRQ)
5. MicroSD, MCP23017 expander, PE4259 RF switch, Wi-Fi/BLE services

Details in [docs/DESIGN.md](docs/DESIGN.md#6-future-expansion-roadmap).

---

## Contributing

This firmware targets the open [Black-Flipper](https://github.com/fussdeek-del/Black-Flipper) hardware project. Read `docs/DESIGN.md` before adding drivers or tasks so new code follows the existing layering and core allocation.

Keep changes focused: skeleton stubs and structure first, real implementations in follow-up PRs.

---

## License

Follow the license of the upstream [Black-Flipper](https://github.com/fussdeek-del/Black-Flipper) hardware repository unless otherwise specified in this repo.
