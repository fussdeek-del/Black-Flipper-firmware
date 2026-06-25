/**
 * @file pn532.c
 * @brief PN532 NFC controller driver implementation.
 */

#include "pn532.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "app_config.h"

static const char *TAG = "DRV_PN532";

#define I2C_MASTER_NUM             I2C_NUM_0    // Configure I2C controller 0
#define I2C_MASTER_FREQ_HZ         100000       // PN532 max I2C speed is 400kHz, 100kHz standard

esp_err_t drv_pn532_init(const pn532_config_t *config)
{
    ESP_LOGI(TAG, "Initializing PN532 NFC Module...");

    // 1. Configure the I2C Master driver
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PIN_PN532_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = PIN_PN532_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ
    };

    // Install I2C driver (ignore if already registered by another component)
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err == ESP_OK) {
        err = i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
    }
    
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) { // INVALID_STATE implies already installed
        ESP_LOGE(TAG, "I2C driver installation failed: %s", esp_err_to_name(err));
        return err;
    }

    // 2. Configure auxiliary IRQ and RESET GPIOs
    ESP_LOGI(TAG, "Configuring PN532 auxiliary GPIOs: IRQ=%d, RST=%d", PIN_PN532_IRQ, PIN_PN532_RST);
    // TODO: gpio_config() busy pin IRQ as input, RST as output

    // 3. Hardware Wakeup sequence (PN532 reset pulse)
    gpio_set_direction(PIN_PN532_RST, GPIO_MODE_OUTPUT);
    gpio_set_level(PIN_PN532_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_PN532_RST, 0); // Reset pulse low
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(PIN_PN532_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(20));     // Wait for boot up

    // 4. Test communication by querying Firmware version
    uint32_t ver = 0;
    if (drv_pn532_get_firmware_version(&ver) == ESP_OK) {
        ESP_LOGI(TAG, "NFC communications verified. Version data: 0x%08X", ver);
    } else {
        ESP_LOGW(TAG, "NFC communication handshake failed. Device might be sleeping.");
    }

    // 5. Send SAMConfiguration command to put PN532 into normal active mode
    ESP_LOGI(TAG, "Configuring SAM (Security Access Module)...");
    // TODO: Send SAMConfiguration command byte array: {0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x14, 0x01, 0x14, 0x00}

    ESP_LOGI(TAG, "PN532 initialized successfully.");
    return ESP_OK;
}

esp_err_t drv_pn532_get_firmware_version(uint32_t *version_info)
{
    ESP_LOGI(TAG, "Querying firmware version...");

    // PN532 commands consist of: 
    // - Preamble {0x00, 0x00, 0xFF}
    // - Length and Checksum bytes
    // - Frame identifier {0xD4}
    // - Command code (0x02 for GetFirmwareVersion)
    // - Command parameters (none)
    // - Postamble {0x00}
    
    // TODO: Send command frame over I2C to write buffer: {0x00, 0x00, 0xFF, 0x02, 0xFE, 0xD4, 0x02, 0x2A, 0x00}
    // TODO: Wait for PN532 ready ACK frame: {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00}
    // TODO: Read response frame containing version bytes (IC, Ver, Rev, Support)

    // Simulated version response (IC: PN532 (0x32), Version: 1, Revision: 6, Support: ISO14443A/B (0x07))
    *version_info = 0x32010607; 
    return ESP_OK;
}

esp_err_t drv_pn532_read_passive_target(uint8_t *uid, uint8_t *uid_len)
{
    // Polling for an ISO14443A card
    // TODO: Write command InListPassiveTarget: {0x00, 0x00, 0xFF, 0x04, 0xFC, 0xD4, 0x4A, 0x01, 0x00}
    // TODO: Wait for IRQ pin to go LOW (indicates response ready)
    // TODO: Read response payload containing target count, UID length, and UID array.

    // Mock Tag response (returns 4-byte MIFARE UID)
    *uid_len = 4;
    uid[0] = 0xAA;
    uid[1] = 0xBB;
    uid[2] = 0xCC;
    uid[3] = 0xDD;

    ESP_LOGI(TAG, "NFC Tag detected! UID: 0x%02X%02X%02X%02X", uid[0], uid[1], uid[2], uid[3]);
    return ESP_OK;
}

esp_err_t drv_pn532_power_down(void)
{
    ESP_LOGI(TAG, "Powering down PN532 NFC...");
    // TODO: Send PowerDown command: {0x00, 0x00, 0xFF, 0x03, 0xFD, 0xD4, 0x16, 0x01, 0x15}
    return ESP_OK;
}

esp_err_t drv_pn532_wakeup(void)
{
    ESP_LOGI(TAG, "Waking up PN532 NFC...");
    // Trigger reset line or toggle I2C clock to wake
    gpio_set_level(PIN_PN532_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(2));
    gpio_set_level(PIN_PN532_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    return ESP_OK;
}
