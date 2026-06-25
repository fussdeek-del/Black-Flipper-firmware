/**
 * @file battery.c
 * @brief TP4056 and ADC battery driver implementation.
 */

#include "battery.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h" // ESP-IDF v5.x ADC OneShot API
#include "app_config.h"

static const char *TAG = "DRV_BATTERY";

// ADC handle and calibration configurations
static adc_oneshot_unit_handle_t adc_handle = NULL;

esp_err_t drv_battery_init(void)
{
    ESP_LOGI(TAG, "Initializing Battery monitoring subsystem...");

    // 1. Configure TP4056 status pins (CHG and STBY) as input with pullup resistors
    ESP_LOGI(TAG, "Configuring TP4056 status GPIOs: CHG: %d, STBY: %d", 
             PIN_TP4056_CHG, PIN_TP4056_STBY);
             
    gpio_config_t status_cfg = {
        .pin_bit_mask = ((1ULL << PIN_TP4056_CHG) | (1ULL << PIN_TP4056_STBY)),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&status_cfg);

    // 2. Configure ADC1 for measuring battery voltage
    ESP_LOGI(TAG, "Configuring ADC1 on channel corresponding to Pin %d", PIN_BAT_ADC);
    
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config, &adc_handle));

    // Configure ADC channel (ADC1 Channel 0, corresponding to GPIO 1 on ESP32-S3)
    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,             // 12 dB attenuation allows measuring up to 3.1V raw
        .bitwidth = ADC_BITWIDTH_12           // 12-bit resolution (0 to 4095)
    };
    // GPIO1 maps to ADC_CHANNEL_0 in ADC1
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_CHANNEL_0, &chan_config));

    // TODO: Initialize ADC calibration schemes (e.g. Line fitting or Curve fitting) if needed.

    ESP_LOGI(TAG, "Battery driver initialized.");
    return ESP_OK;
}

esp_err_t drv_battery_get_voltage(uint32_t *out_voltage_mv)
{
    int raw_val = 0;
    
    // Read raw ADC value
    // In GPIO1 configuration, ADC_CHANNEL_0 is read.
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, ADC_CHANNEL_0, &raw_val));
    
    // Convert raw ADC value to voltage
    // The Flipper Black uses a resistor divider (e.g. 100k / 100k) to scale battery voltage.
    // V_adc = V_batt * 100k / (100k + 100k) = V_batt / 2
    // We multiply raw reading by attenuation factor and divider factor (2.0):
    
    // Simulated voltage calculation:
    // Raw range 0-4095 maps to approx 0-3100 mV at 12dB attenuation.
    float adc_voltage = (raw_val / 4095.0) * 3100.0;
    *out_voltage_mv = (uint32_t)(adc_voltage * 2.0); // Multiply by 2 because of voltage divider

    ESP_LOGD(TAG, "Battery Read: Raw=%d, Voltage=%lu mV", raw_val, *out_voltage_mv);
    return ESP_OK;
}

esp_err_t drv_battery_get_percentage(uint8_t *out_percentage)
{
    uint32_t voltage_mv = 0;
    drv_battery_get_voltage(&voltage_mv);

    // Simple lithium-ion discharge mapping (4.2V is 100%, 3.3V is 0%)
    if (voltage_mv >= 4200) {
        *out_percentage = 100;
    } else if (voltage_mv <= 3300) {
        *out_percentage = 0;
    } else {
        // Linear interpolation helper
        *out_percentage = (uint8_t)(((voltage_mv - 3300) * 100) / (4200 - 3300));
    }

    return ESP_OK;
}

battery_status_t drv_battery_get_status(void)
{
    // TP4056 Pins are active-low (0 indicates pin is pulling low)
    bool chg = (gpio_get_level(PIN_TP4056_CHG) == 0);
    bool stby = (gpio_get_level(PIN_TP4056_STBY) == 0);

    if (chg && !stby) {
        return BATTERY_CHARGING;
    } else if (!chg && stby) {
        return BATTERY_FULL;
    } else if (!chg && !stby) {
        return BATTERY_DISCHARGING;
    }

    return BATTERY_UNKNOWN;
}
