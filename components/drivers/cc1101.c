/**
 * @file cc1101.c
 * @brief CC1101 sub-GHz transceiver driver implementation.
 */

#include "cc1101.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "app_config.h"

static const char *TAG = "DRV_CC1101";

// SPI device handle allocated during registration
static spi_device_handle_t spi_handle = NULL;

esp_err_t drv_cc1101_init(const cc1101_config_t *config)
{
    ESP_LOGI(TAG, "Initializing CC1101 sub-GHz module...");

    // 1. Configure the GPIO pin for Chip Select (CS) and interrupt lines (GDO0, GDO2)
    ESP_LOGI(TAG, "Configuring GPIO CS: %d, GDO0: %d, GDO2: %d", 
             PIN_CC1101_CS, PIN_CC1101_GDO0, PIN_CC1101_GDO2);
    
    // TODO: Use gpio_config() to setup GDO0 and GDO2 as inputs with falling-edge interrupts.
    // TODO: Register an ISR (Interrupt Service Routine) handler for falling-edges on GDO0 (packet ready).

    // 2. SPI device registration on the shared RF SPI bus (pins defined in app_config.h)
    ESP_LOGI(TAG, "Registering CC1101 on RF SPI Bus (SCLK: %d, MISO: %d, MOSI: %d)", 
             PIN_RF_SPI_SCLK, PIN_RF_SPI_MISO, PIN_RF_SPI_MOSI);

    // TODO: spi_bus_add_device(SPI2_HOST, &dev_config, &spi_handle)
    
    // 3. Reset the CC1101 chip by pulsing CS and issuing the SRES (Software Reset) strobe over SPI.
    ESP_LOGI(TAG, "Sending Reset Strobe (SRES) to CC1101");

    // 4. Load configuration parameters: frequency, modulation, baud rate
    if (config) {
        ESP_LOGI(TAG, "Applying configuration: Freq=%u Hz, Modulation=%d, Power=%d dBm",
                 config->frequency_hz, config->modulation, config->tx_power_dbm);
        drv_cc1101_set_frequency(config->frequency_hz);
        // TODO: Write to registers like MDMCFG4, MDMCFG3 to configure baud rate and modulation
    }

    ESP_LOGI(TAG, "CC1101 module initialized successfully.");
    return ESP_OK;
}

esp_err_t drv_cc1101_set_frequency(uint32_t freq_hz)
{
    ESP_LOGI(TAG, "Setting frequency to %u Hz", freq_hz);
    
    // CC1101 frequency calculation: f_carrier = f_osc/2^16 * FREQ[23..0]
    // Assuming a 26 MHz crystal oscillator:
    // FREQ = (freq_hz * 2^16) / 26,000,000
    
    // TODO: Calculate 24-bit value to split into registers FREQ2, FREQ1, FREQ0
    // TODO: Write computed bytes to FREQ2 (0x0D), FREQ1 (0x0E), FREQ0 (0x0F)
    
    return ESP_OK;
}

esp_err_t drv_cc1101_tx_packet(const uint8_t *data, uint8_t len)
{
    if (len > 64) {
        ESP_LOGE(TAG, "Packet length %d exceeds standard 64-byte FIFO limit", len);
        return ESP_ERR_INVALID_SIZE;
    }

    ESP_LOGI(TAG, "Transmitting packet of size %d over sub-GHz...", len);

    // TODO: Strobe SIDLE (0x36) to enter idle mode
    // TODO: Strobe SFTX (0x3B) to flush the TX FIFO buffer
    // TODO: Write packet length and payload data bytes to FIFO register (0x3F)
    // TODO: Strobe STX (0x35) to start transmission
    // TODO: Wait for GDO0 pin to pull high and then low (indicates transmission finished)

    ESP_LOGI(TAG, "TX transmission complete.");
    return ESP_OK;
}

esp_err_t drv_cc1101_rx_packet(uint8_t *data, uint8_t max_len, uint8_t *out_len, int16_t *rssi)
{
    // In a real driver, this is called from the RF task when the GDO0 interrupt fires.
    // For the skeleton, we simulate check of FIFO.
    
    // TODO: Read status register RXBYTES (0x3B | 0xC0) to check how many bytes are in FIFO.
    // If no bytes, return ESP_ERR_NOT_FOUND.
    
    // TODO: Read first byte from FIFO which contains packet length
    // TODO: Read payload bytes from FIFO
    // TODO: Read RSSI and LQI status bytes appended at the end of the packet
    
    // Mock RX fill for testing
    *out_len = 8;
    data[0] = 0xDE; data[1] = 0xAD; data[2] = 0xBE; data[3] = 0xEF;
    data[4] = 0xAA; data[5] = 0x55; data[6] = 0xFF; data[7] = 0x00;
    *rssi = -65; // simulated RSSI in dBm
    
    ESP_LOGI(TAG, "Simulated RX: 8 bytes read from FIFO. RSSI: %d dBm", *rssi);
    return ESP_OK;
}

esp_err_t drv_cc1101_sleep(void)
{
    ESP_LOGI(TAG, "Putting CC1101 to sleep...");
    // TODO: Strobe SIDLE (0x36)
    // TODO: Strobe SPWD (0x39) to power down
    return ESP_OK;
}
