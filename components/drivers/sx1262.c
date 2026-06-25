/**
 * @file sx1262.c
 * @brief SX1262 LoRa transceiver driver implementation.
 */

#include "sx1262.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "rom/ets_sys.h" // For microsecond delay functions
#include "app_config.h"

static const char *TAG = "DRV_SX1262";

static spi_device_handle_t lora_spi_handle = NULL;

bool drv_sx1262_is_busy(void)
{
    // The SX1262 pulls the BUSY line HIGH when it is processing internal operations.
    // We must poll this pin and wait for it to go LOW before any SPI command.
    return gpio_get_level(PIN_SX1262_BUSY) == 1;
}

static void wait_for_ready(void)
{
    // Wait for the BUSY line to go low.
    // In a production driver, a timeout should be implemented to prevent lockups.
    uint32_t timeout = 5000; // 5 ms timeout loop
    while (drv_sx1262_is_busy() && timeout > 0) {
        ets_delay_us(1); // Block for 1 microsecond
        timeout--;
    }
    if (timeout == 0) {
        ESP_LOGE(TAG, "Wait for ready timed out! SX1262 is hung.");
    }
}

esp_err_t drv_sx1262_init(const sx1262_config_t *config)
{
    ESP_LOGI(TAG, "Initializing SX1262 LoRa module...");

    // 1. Configure control GPIOs (BUSY, RST, CS, DIO1)
    ESP_LOGI(TAG, "Configuring control GPIOs: RST: %d, CS: %d, BUSY: %d, DIO1: %d",
             PIN_SX1262_RST, PIN_SX1262_CS, PIN_SX1262_BUSY, PIN_SX1262_DIO1);
             
    // TODO: gpio_config() busy pin as input, RST as output, CS as output (if not using hardware SPI CS control).
    // TODO: Register interrupt on DIO1 for package RX/TX completed notifications.

    // 2. Perform a hardware reset
    ESP_LOGI(TAG, "Performing hardware reset...");
    gpio_set_direction(PIN_SX1262_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_SX1262_RST, 0); // Active low reset
    ets_delay_us(100);                 // Wait 100 microseconds
    gpio_set_level(PIN_SX1262_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(10));     // Wait 10 milliseconds for chip calibration

    // 3. Register on the RF SPI bus
    ESP_LOGI(TAG, "Registering SX1262 on RF SPI Bus...");
    // TODO: spi_bus_add_device(SPI2_HOST, &dev_config, &lora_spi_handle)

    // 4. Base Chip configuration
    wait_for_ready();
    ESP_LOGI(TAG, "SetStandby mode...");
    // TODO: Write command SetStandby (0x80) with Standby_RC argument

    if (config) {
        ESP_LOGI(TAG, "Applying LoRa configurations: Freq=%u Hz, SF=%d, BW=%d",
                 config->frequency_hz, config->spreading_factor, config->bandwidth);
        
        // TODO: Send command SetRfFrequency (0x86) with 4-byte frequency word
        // TODO: Send command SetModulationParams (0x8B) with SF, BW, CodingRate, and LowDataRateOptimize
        // TODO: Send command SetPacketParams (0x8C) with PreambleLength, HeaderType, PayloadLength, CRC
    }

    ESP_LOGI(TAG, "SX1262 module initialized successfully.");
    return ESP_OK;
}

esp_err_t drv_sx1262_tx_packet(const uint8_t *data, uint8_t len)
{
    ESP_LOGI(TAG, "Sending packet of size %d over LoRa...", len);

    wait_for_ready();
    // TODO: Write command SetBufferBaseAddress (0x8F) to set TX/RX offset
    // TODO: Write command WriteBuffer (0x0E) containing payload data
    // TODO: Write command SetTx (0x83) to initiate transmission with timeout
    // TODO: Wait on DIO1 interrupt or poll status for TX Done event

    ESP_LOGI(TAG, "LoRa TX transaction completed.");
    return ESP_OK;
}

esp_err_t drv_sx1262_rx_packet(uint8_t *data, uint8_t max_len, uint8_t *out_len, int16_t *rssi, int8_t *snr)
{
    // Called when DIO1 fires to notify packet RX done.
    
    // TODO: Write command GetRxBufferStatus (0x13) to read start address and length of packet
    // TODO: Write command ReadBuffer (0x1E) to read the packet bytes from FIFO
    // TODO: Write command GetPacketStatus (0x14) to retrieve RSSI and SNR values
    
    // Simulated RX packet
    *out_len = 12;
    for (int i = 0; i < 12; i++) {
        data[i] = 0x10 + i;
    }
    *rssi = -80; // RSSI in dBm
    *snr = 8;    // SNR in dB
    
    ESP_LOGI(TAG, "Simulated LoRa RX: 12 bytes read. RSSI: %d dBm, SNR: %d dB", *rssi, *snr);
    return ESP_OK;
}

esp_err_t drv_sx1262_standby(void)
{
    ESP_LOGI(TAG, "Setting SX1262 to Standby...");
    wait_for_ready();
    // Send SetStandby command
    return ESP_OK;
}
