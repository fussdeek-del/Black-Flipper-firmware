/**
 * @file ui_manager.h
 * @brief User Interface Manager interface.
 */

#pragma once

#include "esp_err.h"

/**
 * @brief Initialize the user interface subsystem (LVGL configuration, themes, input devices).
 */
void ui_manager_init(void);

/**
 * @brief Update the user interface (flushes rendering buffers and handles display events).
 * Must be called periodically (~every 10-30ms) from the UI Core task.
 */
void ui_manager_update(void);
