/**
 * @file pn532.h
 * @brief PN532 NFC controller driver interface.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Supported NFC card/tag types.
 */
typedef enum {
    NFC_CARD_ISO14443A = 0, ///< MIFARE, NTAG, etc.
    NFC_CARD_ISO14443B,
    NFC_CARD_FELICA,
    NFC_CARD_JEWEL
} nfc_card_type_t;

/**
 * @brief NFC Controller configurations.
 */
typedef struct {
    uint8_t i2c_addr;       ///< I2C address (usually 0x48)
    nfc_card_type_t default_card;
} pn532_config_t;

/**
 * @brief Initialize the PN532 chip over the I2C interface.
 * 
 * @param config Configuration parameters
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_pn532_init(const pn532_config_t *config);

/**
 * @brief Query firmware version information from the PN532.
 * 
 * @param version_info Out buffer (must be at least 4 bytes)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_pn532_get_firmware_version(uint32_t *version_info);

/**
 * @brief Put PN532 in passive tag detection mode (ISO14443A).
 * 
 * @param uid Out buffer for card UID
 * @param uid_len Out pointer for UID length
 * @return esp_err_t ESP_OK if a tag was found, ESP_ERR_TIMEOUT if none
 */
esp_err_t drv_pn532_read_passive_target(uint8_t *uid, uint8_t *uid_len);

/**
 * @brief Place the PN532 in low-power Standby mode.
 * 
 * @return esp_err_t ESP_OK
 */
esp_err_t drv_pn532_power_down(void);

/**
 * @brief Wake up the PN532 from Standby mode.
 * 
 * @return esp_err_t ESP_OK
 */
esp_err_t drv_pn532_wakeup(void);
