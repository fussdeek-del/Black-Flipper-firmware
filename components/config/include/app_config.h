/**
 * @file app_config.h
 * @brief Global hardware pinout, FreeRTOS task configuration, and event definitions
 * for the Flipper Black firmware skeleton.
 *
 * This file serves as the central configuration registry. It defines the GPIO pinout,
 * task parameters (priority, stack size, core affinity), and global data structures
 * for inter-core communication.
 *
 * NOTE: GPIO assignments are engineering placeholders until verified against the
 * Black-Flipper KiCad netlist (https://github.com/fussdeek-del/Black-Flipper/tree/main/PCB).
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/* ========================================================================== */
/*                              GPIO PIN ASSIGNMENTS                          */
/* ========================================================================== */

// --- SPI Bus: RF (CC1101 and SX1262) ---
#define PIN_RF_SPI_SCLK         12   // Shared SPI clock
#define PIN_RF_SPI_MISO         13   // Shared Master-In Slave-Out
#define PIN_RF_SPI_MOSI         14   // Shared Master-Out Slave-In

// CC1101 (sub-GHz) specific pins
#define PIN_CC1101_CS           11   // Chip Select (active low)
#define PIN_CC1101_GDO0         10   // General Digital Output 0 (interrupt line)
#define PIN_CC1101_GDO2         9    // General Digital Output 2 (interrupt/carrier sense)

// SX1262 (LoRa) specific pins
#define PIN_SX1262_CS           15   // Chip Select (active low)
#define PIN_SX1262_DIO1         16   // Interrupt line
#define PIN_SX1262_BUSY         17   // Busy status indicator
#define PIN_SX1262_RST          18   // Hardware Reset

// --- SPI Bus: Display and External Flash ---
#define PIN_DISP_SPI_SCLK       37   // Shared SPI clock
#define PIN_DISP_SPI_MOSI       35   // Shared Master-Out Slave-In
#define PIN_DISP_SPI_MISO       36   // Shared Master-In Slave-Out

// ST7796S Display control pins
#define PIN_ST7796S_CS          38   // Display Chip Select
#define PIN_ST7796S_DC          39   // Data/Command selection line
#define PIN_ST7796S_RST         40   // Display Reset
#define PIN_ST7796S_BCKL        41   // Display Backlight control (PWM)

// FT6336U capacitive touch (I2C, on Waveshare 3.5" display module)
#define PIN_TOUCH_I2C_SDA       21   // Touch I2C data (provisional — verify vs schematic)
#define PIN_TOUCH_I2C_SCL       22   // Touch I2C clock (provisional — verify vs schematic)
#define PIN_TOUCH_IRQ           2    // Touch interrupt (active on touch-down)

// External SPI Flash (W25Q128) control pins
#define PIN_W25Q128_CS          47   // Flash Chip Select

// --- NFC Module (PN532) ---
// Communicating over I2C
#define PIN_PN532_SDA           4    // I2C Data line
#define PIN_PN532_SCL           5    // I2C Clock line
#define PIN_PN532_IRQ           6    // NFC Interrupt line
#define PIN_PN532_RST           7    // NFC Reset line

// --- GPS Module (MAX-M8Q) ---
// Communicating over UART2
#define PIN_GPS_TX              43   // TX from ESP32-S3 (connects to MAX-M8Q RX)
#define PIN_GPS_RX              44   // RX to ESP32-S3 (connects to MAX-M8Q TX)

// --- Infrared Transceiver ---
#define PIN_IR_TX               45   // IR Transmitter (uses ESP32-S3 RMT peripheral)
#define PIN_IR_RX               46   // IR Receiver (uses ESP32-S3 RMT peripheral)

// --- Battery and Power Management ---
#define PIN_BAT_ADC             1    // ADC1 Channel 0 for Battery voltage sense
#define PIN_TP4056_CHG          3    // Low indicates active charging
#define PIN_TP4056_STBY         8    // Low indicates fully charged/standby

// --- Unusable/System pins for reference ---
#define PIN_USB_D_MINUS         19   // Built-in ESP32-S3 USB OTG D-
#define PIN_USB_D_PLUS          20   // Built-in ESP32-S3 USB OTG D+
#define PIN_BOOT_STRAP          0    // Hardwired BOOT button

/* Future hardware (no drivers yet): MCP23017 GPIO expander, MicroSD slot,
 * PE4259 RF antenna switch, built-in Wi-Fi/BLE (ESP32-S3 native). */

/* ========================================================================== */
/*                          FREERTOS TASK CONFIGURATION                        */
/* ========================================================================== */

// FreeRTOS CPU Cores on ESP32-S3
#define CORE_ID_RF              0    // PRO CPU (Real-Time RF, time-critical tasks)
#define CORE_ID_SYS             1    // APP CPU (UI, system logic, slow peripherals)

// Task Priorities (higher numbers represent higher priority)
#define PRIORITY_RF_CORE        10   // High priority RF task (Core 0)
#define PRIORITY_IR_CORE        9    // High priority IR signal task (Core 0)
#define PRIORITY_UI_CORE        4    // Medium-low priority UI rendering (Core 1)
#define PRIORITY_SYSTEM_CORE    6    // Medium priority system logic/polling (Core 1)
#define PRIORITY_GPS_CORE       3    // Low priority GPS parsing (Core 1)

// Task Stack Sizes (in bytes - ESP-IDF defines stack sizes in bytes for FreeRTOS)
#define STACK_SIZE_RF_CORE      (4 * 1024)
#define STACK_SIZE_IR_CORE      (3 * 1024)
#define STACK_SIZE_UI_CORE      (8 * 1024)  // UI needs more stack space for display buffer and layout
#define STACK_SIZE_SYSTEM_CORE  (4 * 1024)
#define STACK_SIZE_GPS_CORE     (3 * 1024)

/* ========================================================================== */
/*                        EVENT SYSTEM AND QUEUE CONFIG                        */
/* ========================================================================== */

#define EVENT_QUEUE_LEN         32   // Maximum number of pending events in the queue

/**
 * @brief Global Event Types for Inter-core Communication.
 */
typedef enum {
    EVENT_TYPE_NONE = 0,

    // RF events (GDO/DIO triggers, packet received)
    EVENT_TYPE_RF_SUBGHZ_RX,
    EVENT_TYPE_RF_LORA_RX,
    EVENT_TYPE_RF_TX_COMPLETE,

    // NFC events
    EVENT_TYPE_NFC_TAG_DETECTED,
    EVENT_TYPE_NFC_ERROR,

    // GPS events
    EVENT_TYPE_GPS_FIX_ACQUIRED,
    EVENT_TYPE_GPS_UPDATE,

    // IR events
    EVENT_TYPE_IR_RECEIVED,
    EVENT_TYPE_IR_SENT,

    // Power events
    EVENT_TYPE_BATTERY_LOW,
    EVENT_TYPE_BATTERY_CHARGING,
    EVENT_TYPE_BATTERY_FULL,

    // UI/User input events
    EVENT_TYPE_UI_TOUCH,
    EVENT_TYPE_UI_BUTTON_PRESS,
} app_event_type_t;

/**
 * @brief Event Payload structure.
 */
typedef struct {
    app_event_type_t type;  ///< Type of event occurred
    union {
        // Sub-GHz/LoRa packet details
        struct {
            uint8_t payload[64];
            uint8_t length;
            int16_t rssi;
        } rf_rx;

        // NFC payload details
        struct {
            uint8_t uid[7];
            uint8_t uid_len;
        } nfc_tag;

        // GPS location details
        struct {
            double latitude;
            double longitude;
            float speed_kmh;
            uint8_t sat_count;
        } gps;

        // Infrared signal code
        struct {
            uint32_t protocol;
            uint64_t address_data;
        } ir;

        // Battery capacity details
        struct {
            float voltage;
            uint8_t percentage;
        } battery;

        // Keypad/button index
        struct {
            uint8_t key_code;
            bool is_pressed;
        } key;
    } data;
} app_event_t;
