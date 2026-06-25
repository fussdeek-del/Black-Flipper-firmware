/**
 * @file w25q128.h
 * @brief W25Q128 16MB external SPI Flash driver interface.
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"

/**
 * @brief Initialize the W25Q128 flash and mount the partition as a FAT/SPIFFS filesystem.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_w25q128_init(void);

/**
 * @brief Write raw data block to flash address.
 * 
 * @param addr Destination flash physical offset
 * @param buffer Source buffer
 * @param size Byte size of write
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_w25q128_write(uint32_t addr, const uint8_t *buffer, uint32_t size);

/**
 * @brief Read raw data block from flash address.
 * 
 * @param addr Source flash physical offset
 * @param buffer Destination buffer
 * @param size Byte size of read
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_w25q128_read(uint32_t addr, uint8_t *buffer, uint32_t size);

/**
 * @brief Sector erase (4KB sectors).
 * 
 * @param sector_addr Sector physical start address
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_w25q128_erase_sector(uint32_t sector_addr);

/**
 * @brief Mount FATFS / LittleFS on the flash chip to expose file storage.
 * 
 * @param mount_point Directory prefix (e.g. "/flash")
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_w25q128_mount_filesystem(const char *mount_point);
