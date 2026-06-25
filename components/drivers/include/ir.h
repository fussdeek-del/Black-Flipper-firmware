/**
 * @file ir.h
 * @brief Infrared transmitter and receiver driver interface.
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

/**
 * @brief Supported IR protocols.
 */
typedef enum {
    IR_PROTO_NEC = 0,
    IR_PROTO_RC5,
    IR_PROTO_RC6,
    IR_PROTO_UNKNOWN
} ir_protocol_t;

/**
 * @brief Struct representing a decoded IR frame.
 */
typedef struct {
    ir_protocol_t protocol;
    uint32_t address;
    uint64_t command;
} ir_packet_t;

/**
 * @brief Initialize the Infrared hardware subsystem using the ESP32-S3 RMT peripheral.
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_ir_init(void);

/**
 * @brief Transmit a code over IR.
 * 
 * @param protocol IR protocol type (e.g. NEC)
 * @param address Chip address
 * @param command Command payload
 * @return esp_err_t ESP_OK on success
 */
esp_err_t drv_ir_tx(ir_protocol_t protocol, uint32_t address, uint64_t command);

/**
 * @brief Read/decode received IR packet if available.
 * 
 * @param out_packet Decoded packet destination
 * @return esp_err_t ESP_OK if packet decoded, ESP_ERR_NOT_FOUND if none in queue
 */
esp_err_t drv_ir_rx(ir_packet_t *out_packet);
