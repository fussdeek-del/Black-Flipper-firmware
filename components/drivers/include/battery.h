/**
 * @file battery.h
 * @brief TP4056 and ADC battery driver interface.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Charger state.
 */
typedef enum {
    BATTERY_DISCHARGING = 0,
    BATTERY_CHARGING,
    BATTERY_FULL,
    BATTERY_UNKNOWN
} battery_status_t;

/**
 * @brief Initialize battery management pins (ADC, TP4056 status pins).
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_battery_init(void);

/**
 * @brief Get battery voltage in millivolts.
 * 
 * @param out_voltage_mv Pointer to write the voltage value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_battery_get_voltage(uint32_t *out_voltage_mv);

/**
 * @brief Get battery percentage (0-100%).
 * 
 * @param out_percentage Pointer to write percentage value
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_battery_get_percentage(uint8_t *out_percentage);

/**
 * @brief Get charging status.
 * 
 * @return battery_status_t Current charger chip status
 */
battery_status_t drv_battery_get_status(void);
