/**
 * @file st7796s.h
 * @brief ST7796S 3.5" LCD controller driver interface.
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"

// Display resolution definitions
#define LCD_WIDTH               320
#define LCD_HEIGHT              480

/**
 * @brief Initialize the ST7796S display controller over SPI.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_st7796s_init(void);

/**
 * @brief Write a block of pixels (color buffer) to a specified screen window.
 * This is the standard flush callback required by LVGL.
 * 
 * @param x_start Starting X coordinate
 * @param y_start Starting Y coordinate
 * @param x_end Ending X coordinate
 * @param y_end Ending Y coordinate
 * @param color_map Color buffer array (usually 16-bit RGB565)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_st7796s_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t *color_map);

/**
 * @brief Set display backlight brightness level.
 * 
 * @param level Brightness level from 0 (off) to 100 (full brightness)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_st7796s_set_backlight(uint8_t level);

/**
 * @brief Turn display output off (sleep mode).
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_st7796s_sleep(void);

/**
 * @brief Wake up display output.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_st7796s_wakeup(void);
