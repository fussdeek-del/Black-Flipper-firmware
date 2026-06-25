/**
 * @file max_m8q.h
 * @brief MAX-M8Q GPS receiver driver interface.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief GPS Data Structure.
 */
typedef struct {
    double latitude;        ///< Latitude in decimal degrees
    double longitude;       ///< Longitude in decimal degrees
    float altitude_m;       ///< Altitude in meters above sea level
    float speed_kmh;        ///< Speed in kilometers per hour
    float course_deg;       ///< Course heading in degrees
    uint8_t satellites;     ///< Number of satellites in view/use
    bool has_fix;           ///< Fix status
    struct {
        uint8_t hour;
        uint8_t minute;
        uint8_t second;
    } time;
} gps_data_t;

/**
 * @brief Initialize the MAX-M8Q GPS driver UART interface.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_max_m8q_init(void);

/**
 * @brief Read and parse raw NMEA streams from GPS UART.
 * 
 * @param out_data Output data struct populated with parsed values
 * @return esp_err_t ESP_OK if new valid location data is parsed, ESP_ERR_NOT_FOUND if no new data
 */
esp_err_t drv_max_m8q_read(gps_data_t *out_data);

/**
 * @brief Set the GPS into Standby/Power-Save mode.
 * 
 * @return esp_err_t ESP_OK
 */
esp_err_t drv_max_m8q_sleep(void);

/**
 * @brief Wake the GPS from power save mode.
 * 
 * @return esp_err_t ESP_OK
 */
esp_err_t drv_max_m8q_wakeup(void);
