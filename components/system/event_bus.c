/**
 * @file event_bus.c
 * @brief Inter-core Event Bus implementation.
 */

#include "event_bus.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

static const char *TAG = "SYS_EVENT_BUS";

// FreeRTOS queue handle
static QueueHandle_t global_event_queue = NULL;

esp_err_t event_bus_init(void)
{
    if (global_event_queue != NULL) {
        ESP_LOGW(TAG, "Event Bus already initialized.");
        return ESP_OK;
    }

    // Create the queue capable of holding EVENT_QUEUE_LEN (32) items of app_event_t size.
    // Memory is allocated dynamically from the ESP32-S3's internal SRAM.
    global_event_queue = xQueueCreate(EVENT_QUEUE_LEN, sizeof(app_event_t));
    
    if (global_event_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create FreeRTOS Queue!");
        return ESP_ERR_NO_MEM;
    }

    ESP_LOGI(TAG, "FreeRTOS event queue created (Length: %d, Item Size: %d bytes).", 
             EVENT_QUEUE_LEN, (int)sizeof(app_event_t));
             
    return ESP_OK;
}

esp_err_t event_bus_post(const app_event_t *event)
{
    if (global_event_queue == NULL) {
        ESP_LOGE(TAG, "Cannot post event: Event Bus queue is NULL.");
        return ESP_ERR_INVALID_STATE;
    }

    // Post/Enqueue an item to the back of the queue.
    // Parameters: Queue handle, pointer to data to copy, timeout ticks (0 = non-blocking).
    // Using 0 ticks makes it safe to call from time-critical loops as it will return immediately
    // rather than blocking if the queue is full.
    BaseType_t ret = xQueueSend(global_event_queue, event, 0);
    
    if (ret != pdPASS) {
        ESP_LOGW(TAG, "Event Queue full! Event of type %d dropped.", event->type);
        return ESP_ERR_TIMEOUT;
    }
    
    return ESP_OK;
}

esp_err_t event_bus_receive(app_event_t *out_event, uint32_t timeout_ticks)
{
    if (global_event_queue == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    // Dequeue an item from the front of the queue.
    // Parameters: Queue handle, pointer to output buffer, timeout ticks.
    // This blocks the calling task until an item is available or timeout expires.
    BaseType_t ret = xQueueReceive(global_event_queue, out_event, timeout_ticks);
    
    if (ret != pdPASS) {
        return ESP_ERR_TIMEOUT;
    }
    
    return ESP_OK;
}
