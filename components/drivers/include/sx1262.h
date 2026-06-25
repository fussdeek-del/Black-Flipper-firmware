/**
 * @file sx1262.h
 * @brief SX1262 LoRa transceiver driver interface.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief SX1262 LoRa Spreading Factors.
 */
typedef enum {
    LORA_SF5 = 5,
    LORA_SF6,
    LORA_SF7,
    LORA_SF8,
    LORA_SF9,
    LORA_SF10,
    LORA_SF11,
    LORA_SF12
} sx1262_sf_t;

/**
 * @brief SX1262 LoRa Bandwidths.
 */
typedef enum {
    LORA_BW_7_8_KHZ = 0,
    LORA_BW_15_6_KHZ,
    LORA_BW_31_25_KHZ,
    LORA_BW_62_5_KHZ,
    LORA_BW_125_KHZ,
    LORA_BW_250_KHZ,
    LORA_BW_500_KHZ
} sx1262_bw_t;

/**
 * @brief SX1262 Configuration structure.
 */
typedef struct {
    uint32_t frequency_hz;          ///< Center frequency (e.g. 868100000 or 915000000)
    sx1262_sf_t spreading_factor;   ///< Spreading Factor (SF5 to SF12)
    sx1262_bw_t bandwidth;          ///< Bandwidth
    uint8_t coding_rate;            ///< Coding Rate (e.g., 4/5, 4/6, etc., represented as 1 to 4)
    int8_t tx_power_dbm;            ///< Output power in dBm (up to +22 dBm)
} sx1262_config_t;

/**
 * @brief Initialize the SX1262 LoRa chip and peripheral pins.
 * 
 * @param config Configuration parameters
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_sx1262_init(const sx1262_config_t *config);

/**
 * @brief Transmit a packet over LoRa.
 * 
 * @param data Buffer containing payload to transmit
 * @param len Byte size of transmission payload
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_sx1262_tx_packet(const uint8_t *data, uint8_t len);

/**
 * @brief Attempt to read a LoRa packet from the FIFO.
 * 
 * @param data Out buffer for payload
 * @param max_len Size of output buffer
 * @param out_len Pointer to write actual read length
 * @param rssi Pointer to write RSSI measurement in dBm
 * @param snr Pointer to write Signal-to-Noise ratio
 * @return esp_err_t ESP_OK on success, ESP_ERR_NOT_FOUND if no packet
 */
esp_err_t drv_sx1262_rx_packet(uint8_t *data, uint8_t max_len, uint8_t *out_len, int16_t *rssi, int8_t *snr);

/**
 * @brief Check if SX1262 is busy.
 * 
 * @return true Chip is busy
 * @return false Chip is ready
 */
bool drv_sx1262_is_busy(void);

/**
 * @brief Set the SX1262 to Standby mode.
 * 
 * @return esp_err_t ESP_OK
 */
esp_err_t drv_sx1262_standby(void);
