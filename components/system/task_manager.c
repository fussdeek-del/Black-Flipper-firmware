/**
 * @file task_manager.c
 * @brief Core task loops and orchestration for Flipper Black.
 */

#include "task_manager.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

// Include driver interfaces
#include "cc1101.h"
#include "sx1262.h"
#include "pn532.h"
#include "max_m8q.h"
#include "st7796s.h"
#include "ir.h"
#include "w25q128.h"
#include "battery.h"

// Include event system
#include "event_bus.h"
#include "app_config.h"

static const char *TAG_RF = "SYS_TASK_RF";
static const char *TAG_SYS = "SYS_TASK_CORE";
static const char *TAG_GPS = "SYS_TASK_GPS";
static const char *TAG_IR = "SYS_TASK_IR";

void rf_core_task_entry(void *pvParameters)
{
    ESP_LOGI(TAG_RF, "RF Task started on CPU Core: %d", xPortGetCoreID());

    // 1. Initialize RF Drivers
    cc1101_config_t subghz_config = {
        .frequency_hz = 433920000,
        .baud_rate = 4800,
        .modulation = CC1101_MOD_ASK_OOK,
        .tx_power_dbm = 10
    };
    drv_cc1101_init(&subghz_config);

    sx1262_config_t lora_config = {
        .frequency_hz = 915000000,
        .spreading_factor = LORA_SF7,
        .bandwidth = LORA_BW_125_KHZ,
        .coding_rate = 1, // 4/5
        .tx_power_dbm = 14
    };
    drv_sx1262_init(&lora_config);

    // 2. Continuous polling loop
    uint8_t buffer[64];
    uint8_t rx_len = 0;
    int16_t rssi = 0;
    int8_t snr = 0;

    while (1) {
        // --- CC1101 Check ---
        // In a real firmware, we would wait for a FreeRTOS Notification sent from the GPIO ISR.
        // For the skeleton, we poll or block briefly to simulate processing.
        if (drv_cc1101_rx_packet(buffer, sizeof(buffer), &rx_len, &rssi) == ESP_OK) {
            app_event_t event = {
                .type = EVENT_TYPE_RF_SUBGHZ_RX
            };
            event.data.rf_rx.length = rx_len;
            event.data.rf_rx.rssi = rssi;
            memcpy(event.data.rf_rx.payload, buffer, rx_len);
            
            // Push event to system cores
            event_bus_post(&event);
        }

        // --- SX1262 Check ---
        if (drv_sx1262_rx_packet(buffer, sizeof(buffer), &rx_len, &rssi, &snr) == ESP_OK) {
            app_event_t event = {
                .type = EVENT_TYPE_RF_LORA_RX
            };
            event.data.rf_rx.length = rx_len;
            event.data.rf_rx.rssi = rssi;
            memcpy(event.data.rf_rx.payload, buffer, rx_len);
            
            event_bus_post(&event);
        }

        // Sleep to yield CPU control (simulates low power wait state)
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void ir_core_task_entry(void *pvParameters)
{
    ESP_LOGI(TAG_IR, "IR Task started on CPU Core: %d", xPortGetCoreID());
    
    drv_ir_init();
    
    ir_packet_t rx_pkt;

    while (1) {
        // Simulate waiting for IR pulse signals
        vTaskDelay(pdMS_TO_TICKS(5000)); // Every 5 seconds, simulate an IR reception

        if (drv_ir_rx(&rx_pkt) == ESP_OK) {
            app_event_t event = {
                .type = EVENT_TYPE_IR_RECEIVED
            };
            event.data.ir.protocol = rx_pkt.protocol;
            event.data.ir.address_data = ((uint64_t)rx_pkt.address << 32) | rx_pkt.command;
            
            event_bus_post(&event);
        }
    }
}

void system_core_task_entry(void *pvParameters)
{
    ESP_LOGI(TAG_SYS, "System Manager Task started on CPU Core: %d", xPortGetCoreID());

    // 1. Initialize Slow drivers
    pn532_config_t nfc_cfg = {
        .i2c_addr = 0x48,
        .default_card = NFC_CARD_ISO14443A
    };
    drv_pn532_init(&nfc_cfg);
    drv_battery_init();

    // 2. System main loop
    uint8_t nfc_uid[7];
    uint8_t nfc_len = 0;
    
    uint32_t battery_mv = 0;
    uint8_t battery_pct = 0;
    uint32_t last_bat_check = 0;

    while (1) {
        // --- NFC Poll ---
        // NFC poll blocks for 100ms. Keep timeout low so we don't starve the task loop.
        if (drv_pn532_read_passive_target(nfc_uid, &nfc_len) == ESP_OK) {
            app_event_t event = {
                .type = EVENT_TYPE_NFC_TAG_DETECTED
            };
            event.data.nfc_tag.uid_len = nfc_len;
            memcpy(event.data.nfc_tag.uid, nfc_uid, nfc_len);
            
            event_bus_post(&event);
        }

        // --- Battery Voltage Poll ---
        uint32_t now = xTaskGetTickCount() * portTICK_PERIOD_MS;
        if (now - last_bat_check > 10000 || last_bat_check == 0) { // Every 10 seconds
            last_bat_check = now;
            drv_battery_get_voltage(&battery_mv);
            drv_battery_get_percentage(&battery_pct);
            battery_status_t state = drv_battery_get_status();

            ESP_LOGI(TAG_SYS, "Battery Check: %lu mV (%d%%), Status: %d",
                     (unsigned long)battery_mv, battery_pct, state);

            app_event_t event;
            if (state == BATTERY_CHARGING) {
                event.type = EVENT_TYPE_BATTERY_CHARGING;
            } else if (state == BATTERY_FULL) {
                event.type = EVENT_TYPE_BATTERY_FULL;
            } else if (battery_pct < 15) {
                event.type = EVENT_TYPE_BATTERY_LOW;
            } else {
                event.type = EVENT_TYPE_NONE;
            }

            if (event.type != EVENT_TYPE_NONE) {
                event.data.battery.voltage = battery_mv / 1000.0f;
                event.data.battery.percentage = battery_pct;
                event_bus_post(&event);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // Period check yield
    }
}

void gps_core_task_entry(void *pvParameters)
{
    ESP_LOGI(TAG_GPS, "GPS Task started on CPU Core: %d", xPortGetCoreID());

    drv_max_m8q_init();

    gps_data_t gps_frame;

    while (1) {
        // Read GPS stream from UART ring buffer
        if (drv_max_m8q_read(&gps_frame) == ESP_OK) {
            if (gps_frame.has_fix) {
                app_event_t event = {
                    .type = EVENT_TYPE_GPS_UPDATE
                };
                event.data.gps.latitude = gps_frame.latitude;
                event.data.gps.longitude = gps_frame.longitude;
                event.data.gps.speed_kmh = gps_frame.speed_kmh;
                event.data.gps.sat_count = gps_frame.satellites;
                
                event_bus_post(&event);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(2000)); // GPS usually outputs at 1Hz rate, poll every 2 seconds
    }
}

void ui_core_task_entry(void *pvParameters)
{
    ESP_LOGI(TAG_SYS, "UI Render Task started on CPU Core: %d", xPortGetCoreID());

    // 1. Initialize storage and screen display
    drv_w25q128_init();
    drv_w25q128_mount_filesystem("/flash");
    
    drv_st7796s_init();

    // 2. Spawn / Initialize UI components
    // Import from UI component manager
    extern void ui_manager_init(void);
    extern void ui_manager_update(void);

    ui_manager_init();

    while (1) {
        // Run UI Loop (LVGL task handler needs periodic calls to execute timers)
        ui_manager_update();
        
        vTaskDelay(pdMS_TO_TICKS(20)); // ~50 FPS target refresh
    }
}
