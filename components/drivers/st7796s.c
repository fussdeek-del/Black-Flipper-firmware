/**
 * @file st7796s.c
 * @brief ST7796S 3.5" LCD controller driver implementation.
 */

#include "st7796s.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h" // For display backlight PWM control
#include "app_config.h"

static const char *TAG = "DRV_ST7796S";

// SPI handle for display transactions
static spi_device_handle_t lcd_spi_handle = NULL;

// Helper to write a command byte to the ST7796S
static void lcd_write_cmd(uint8_t cmd)
{
    // DC pin LOW indicates command byte
    gpio_set_level(PIN_ST7796S_DC, 0);
    
    // TODO: spi_device_transmit(lcd_spi_handle, &transaction)
}

// Helper to write data bytes to the ST7796S
static void lcd_write_data(const uint8_t *data, int len)
{
    // DC pin HIGH indicates data bytes
    gpio_set_level(PIN_ST7796S_DC, 1);
    
    // TODO: spi_device_transmit(lcd_spi_handle, &transaction)
}

esp_err_t drv_st7796s_init(void)
{
    ESP_LOGI(TAG, "Initializing ST7796S 3.5\" LCD Controller...");

    // 1. Configure control GPIOs (CS, DC, RST)
    ESP_LOGI(TAG, "Configuring Display control pins: CS: %d, DC: %d, RST: %d", 
             PIN_ST7796S_CS, PIN_ST7796S_DC, PIN_ST7796S_RST);
             
    // TODO: Use gpio_config() to setup RST, CS, DC pins as push-pull outputs.

    // 2. Hardware reset the LCD panel
    ESP_LOGI(TAG, "Resetting display hardware...");
    gpio_set_level(PIN_ST7796S_RST, 0);
    vTaskDelay(pdMS_TO_TICKS(20));
    gpio_set_level(PIN_ST7796S_RST, 1);
    vTaskDelay(pdMS_TO_TICKS(120));

    // 3. Register LCD on the display SPI bus (SPI3 on ESP32-S3)
    ESP_LOGI(TAG, "Registering ST7796S on Display SPI Bus (SCLK: %d, MOSI: %d, MISO: %d)",
             PIN_DISP_SPI_SCLK, PIN_DISP_SPI_MOSI, PIN_DISP_SPI_MISO);
             
    // TODO: spi_bus_add_device(SPI3_HOST, &dev_cfg, &lcd_spi_handle)

    // 4. Initialize Backlight PWM using LEDC Peripheral
    ESP_LOGI(TAG, "Configuring Backlight LEDC PWM on pin %d", PIN_ST7796S_BCKL);
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_8_BIT, // 0 to 255
        .freq_hz          = 5000,              // 5 kHz frequency
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = PIN_ST7796S_BCKL,
        .duty           = 0, // initially off
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);

    // 5. Send ST7796S initialization commands sequence
    ESP_LOGI(TAG, "Sending ST7796S register commands...");
    
    // TODO: Write Sleep Out command (0x11)
    lcd_write_cmd(0x11);
    vTaskDelay(pdMS_TO_TICKS(120));
    
    // TODO: Write Interface Pixel Format command (0x3A) setting 16-bit RGB565 (0x55)
    uint8_t pix_fmt = 0x55;
    lcd_write_cmd(0x3A);
    lcd_write_data(&pix_fmt, 1);
    
    // TODO: Write Display On command (0x29)
    lcd_write_cmd(0x29);
    
    // 6. Turn on Backlight to 80% duty cycle
    drv_st7796s_set_backlight(80);

    ESP_LOGI(TAG, "Display initialization sequence sent. Screen active.");
    return ESP_OK;
}

esp_err_t drv_st7796s_flush(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end, const uint16_t *color_map)
{
    // Write window column addresses (0x2A) and row addresses (0x2B)
    // TODO: 
    // uint8_t cols[4] = { x_start >> 8, x_start & 0xFF, x_end >> 8, x_end & 0xFF };
    // lcd_write_cmd(0x2A);
    // lcd_write_data(cols, 4);
    // uint8_t rows[4] = { y_start >> 8, y_start & 0xFF, y_end >> 8, y_end & 0xFF };
    // lcd_write_cmd(0x2B);
    // lcd_write_data(rows, 4);
    
    // Write pixel memory (0x2C)
    // lcd_write_cmd(0x2C);
    // uint32_t size = (x_end - x_start + 1) * (y_end - y_start + 1) * sizeof(uint16_t);
    // lcd_write_data((const uint8_t*)color_map, size);

    ESP_LOGD(TAG, "LCD Flush: Window[%d,%d to %d,%d]", x_start, y_start, x_end, y_end);
    return ESP_OK;
}

esp_err_t drv_st7796s_set_backlight(uint8_t level)
{
    if (level > 100) level = 100;
    
    // Convert percentage (0-100) to 8-bit duty cycle (0-255)
    uint32_t duty = (level * 255) / 100;
    
    ESP_LOGI(TAG, "Setting Backlight level to %d%% (Duty: %lu/255)", level, duty);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    
    return ESP_OK;
}

esp_err_t drv_st7796s_sleep(void)
{
    ESP_LOGI(TAG, "Putting screen to sleep...");
    drv_st7796s_set_backlight(0); // turn off backlight
    lcd_write_cmd(0x28);          // Display Off
    lcd_write_cmd(0x10);          // Sleep In
    return ESP_OK;
}

esp_err_t drv_st7796s_wakeup(void)
{
    ESP_LOGI(TAG, "Waking up screen...");
    lcd_write_cmd(0x11);          // Sleep Out
    vTaskDelay(pdMS_TO_TICKS(120));
    lcd_write_cmd(0x29);          // Display On
    drv_st7796s_set_backlight(80); // restore backlight
    return ESP_OK;
}
