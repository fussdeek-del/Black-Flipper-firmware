/**
 * @file cc1101.h
 * @brief CC1101 sub-GHz transceiver driver interface.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief CC1101 Modulations.
 */
typedef enum {
    CC1101_MOD_ASK_OOK = 0,
    CC1101_MOD_2FSK,
    CC1101_MOD_GFSK,
    CC1101_MOD_MSK
} cc1101_modulation_t;

/**
 * @brief CC1101 Configuration options.
 */
typedef struct {
    uint32_t frequency_hz;          ///< RF Operating Frequency (e.g. 433920000)
    uint32_t baud_rate;             ///< Data speed symbol rate
    cc1101_modulation_t modulation; ///< Modulation scheme
    int8_t tx_power_dbm;            ///< Output power in dBm
} cc1101_config_t;

/**
 * @brief Initialize the CC1101 hardware interface (GPIO, SPI pins).
 * 
 * @param config Pointer to the configuration struct
 * @return esp_err_t ESP_OK on success, or an error code
 */
esp_err_t drv_cc1101_init(const cc1101_config_t *config);

/**
 * @brief Set operating frequency of CC1101.
 * 
 * @param freq_hz Target frequency in Hz
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_cc1101_set_frequency(uint32_t freq_hz);

/**
 * @brief Transmit a raw packet over sub-GHz.
 * 
 * @param data Data buffer to send
 * @param len Size of data in bytes
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_cc1101_tx_packet(const uint8_t *data, uint8_t len);

/**
 * @brief Read a received packet from CC1101 FIFO if available.
 * 
 * @param data Out buffer for packet data
 * @param max_len Size of out buffer
 * @param out_len Pointer to store actual read packet size
 * @param rssi Pointer to store RSSI of received packet
 * @return esp_err_t ESP_OK on success, or ESP_ERR_NOT_FOUND if no packet in FIFO
 */
esp_err_t drv_cc1101_rx_packet(uint8_t *data, uint8_t max_len, uint8_t *out_len, int16_t *rssi);

/**
 * @brief Set the chip to low-power Sleep mode.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_cc1101_sleep(void);
