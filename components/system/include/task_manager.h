/**
 * @file task_manager.h
 * @brief Core task entries and coordination headers.
 */

#pragma once

/**
 * @brief Task entry for Core 0 (RF Core).
 * Handles the CC1101 and SX1262 SPI polling/interrupt loops.
 * 
 * @param pvParameters Parameter pointer (NULL)
 */
void rf_core_task_entry(void *pvParameters);

/**
 * @brief Task entry for Core 0 (IR sub-task).
 * Processes tight timing carrier signals for IR Tx/Rx.
 * 
 * @param pvParameters Parameter pointer (NULL)
 */
void ir_core_task_entry(void *pvParameters);

/**
 * @brief Task entry for Core 1 (System Core).
 * Orchestrates NFC checks, power calculations, and overall device state changes.
 * 
 * @param pvParameters Parameter pointer (NULL)
 */
void system_core_task_entry(void *pvParameters);

/**
 * @brief Task entry for Core 1 (UI Core).
 * Dedicated UI polling, display buffer flushing, and touch input processing.
 * 
 * @param pvParameters Parameter pointer (NULL)
 */
void ui_core_task_entry(void *pvParameters);

/**
 * @brief Task entry for Core 1 (GPS Core).
 * Processes GPS UART characters, parses lines, and dispatches location changes.
 * 
 * @param pvParameters Parameter pointer (NULL)
 */
void gps_core_task_entry(void *pvParameters);
