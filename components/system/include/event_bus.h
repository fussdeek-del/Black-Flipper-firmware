/**
 * @file event_bus.h
 * @brief Inter-core Event Bus interface.
 */

#pragma once

#include "esp_err.h"
#include "app_config.h" // Holds app_event_t struct definition

/**
 * @brief Initialize the global Event Bus queue.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t event_bus_init(void);

/**
 * @brief Post an event to the Event Bus (Thread-safe, can be called from any core/task).
 * 
 * @param event Pointer to the event packet to enqueue
 * @return esp_err_t ESP_OK if enqueued successfully, ESP_ERR_TIMEOUT if queue is full
 */
esp_err_t event_bus_post(const app_event_t *event);

/**
 * @brief Receive/Dequeue an event from the Event Bus.
 * 
 * @param out_event Destination structure to write the dequeued event
 * @param timeout_ticks Number of FreeRTOS ticks to wait for an event (portMAX_DELAY to block)
 * @return esp_err_t ESP_OK if event dequeued, ESP_ERR_TIMEOUT if timer expired
 */
esp_err_t event_bus_receive(app_event_t *out_event, uint32_t timeout_ticks);
