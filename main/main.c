/**
 * @file main.c
 * @brief Firmware entry point for Flipper Black.
 * 
 * This file contains the app_main() function. It initializes NVS (Non-Volatile Storage)
 * which is needed by Wi-Fi/Bluetooth stack and drivers, boots the inter-core event bus,
 * and spawns the primary tasks pinned to Core 0 (PRO CPU) and Core 1 (APP CPU).
 */

#include <stdio.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_config.h"

// Include System manager component headers
#include "task_manager.h"
#include "event_bus.h"

// Log tag for the main application startup
static const char *TAG = "MAIN_APP";

void app_main(void)
{
    ESP_LOGI(TAG, "===============================================");
    ESP_LOGI(TAG, "      Flipper Black Handheld Firmware Booting  ");
    ESP_LOGI(TAG, "===============================================");
    ESP_LOGI(TAG, "Device Hardware: ESP32-S3 (QFN56)");
    ESP_LOGI(TAG, "Firmware Skeleton initializing...");

    // 1. Initialize NVS (Non-Volatile Storage). ESP-IDF uses NVS to save settings and calibration.
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // If NVS partition was truncated or format changed, erase and reinitialize it
        ESP_LOGW(TAG, "NVS flash corrupted. Erasing partition to recover...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(TAG, "NVS Flash Initialized successfully.");

    // 2. Initialize the Inter-Core Event Bus.
    // This creates the FreeRTOS queues and setups message passing between cores.
    ESP_LOGI(TAG, "Initializing Event Bus...");
    if (event_bus_init() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Event Bus! Halting system.");
        return;
    }
    ESP_LOGI(TAG, "Event Bus initialized.");

    // 3. Spawn tasks on CPU Core 0 (PRO CPU) - Dedicated to time-critical RF operations
    ESP_LOGI(TAG, "Spawning tasks on Core 0 (RF CPU)...");

    // Creating the real-time RF task
    // Parameters: Task entry function, Task name, Stack size (words/bytes depending on config, bytes in ESP-IDF), 
    //             Parameters to task (NULL), Priority, Task handle, Core ID
    xTaskCreatePinnedToCore(
        rf_core_task_entry,       // Function that runs the task code
        "RF_Core_Task",           // Text name for debugging
        STACK_SIZE_RF_CORE,       // Stack size in bytes
        NULL,                     // Parameters passed to the task
        PRIORITY_RF_CORE,         // Priority of the task
        NULL,                     // Task handle (not storing one)
        CORE_ID_RF                // Pin task to Core 0 (PRO CPU)
    );

    // Creating the Infrared transceiver task
    xTaskCreatePinnedToCore(
        ir_core_task_entry,
        "IR_Core_Task",
        STACK_SIZE_IR_CORE,
        NULL,
        PRIORITY_IR_CORE,
        NULL,
        CORE_ID_RF                // Pin IR task to Core 0 as it relies on tight carrier timing
    );

    // 4. Spawn tasks on CPU Core 1 (APP CPU) - Dedicated to UI and background system logic
    ESP_LOGI(TAG, "Spawning tasks on Core 1 (System / APP CPU)...");

    // Creating the main background System Logic task
    xTaskCreatePinnedToCore(
        system_core_task_entry,
        "System_Core_Task",
        STACK_SIZE_SYSTEM_CORE,
        NULL,
        PRIORITY_SYSTEM_CORE,
        NULL,
        CORE_ID_SYS               // Pin task to Core 1 (APP CPU)
    );

    // Creating the User Interface and Graphic task
    xTaskCreatePinnedToCore(
        ui_core_task_entry,
        "UI_Core_Task",
        STACK_SIZE_UI_CORE,
        NULL,
        PRIORITY_UI_CORE,
        NULL,
        CORE_ID_SYS               // Pin UI rendering and user interactions to Core 1
    );

    // Creating the GPS polling task
    xTaskCreatePinnedToCore(
        gps_core_task_entry,
        "GPS_Core_Task",
        STACK_SIZE_GPS_CORE,
        NULL,
        PRIORITY_GPS_CORE,
        NULL,
        CORE_ID_SYS               // Pin GPS RX processing to Core 1
    );

    ESP_LOGI(TAG, "All cores booted. Scheduler running. Main task exiting...");
    // app_main runs under a low priority main task which can safely end or yield.
}
