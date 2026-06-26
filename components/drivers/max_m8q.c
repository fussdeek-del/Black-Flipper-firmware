/**
 * @file max_m8q.c
 * @brief MAX-M8Q GPS receiver driver implementation.
 */

#include "max_m8q.h"
#include "esp_log.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_config.h"

static const char *TAG = "DRV_MAX_M8Q";

// Use UART port 2 for GPS communication
#define GPS_UART_NUM            UART_NUM_2
#define GPS_BUF_SIZE            1024

esp_err_t drv_max_m8q_init(void)
{
    ESP_LOGI(TAG, "Initializing MAX-M8Q GPS module...");

    // 1. Configure the UART parameters
    uart_config_t uart_config = {
        .baud_rate = 9600,             // MAX-M8Q default baud rate is 9600
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Configure UART parameters
    ESP_ERROR_CHECK(uart_param_config(GPS_UART_NUM, &uart_config));

    // Set UART pins (TX, RX, RTS/CTS - none)
    ESP_ERROR_CHECK(uart_set_pin(GPS_UART_NUM, PIN_GPS_TX, PIN_GPS_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    // Install UART driver with TX/RX ring buffers
    ESP_ERROR_CHECK(uart_driver_install(GPS_UART_NUM, GPS_BUF_SIZE * 2, 0, 0, NULL, 0));

    ESP_LOGI(TAG, "GPS UART (port %d) initialized at 9600 baud.", GPS_UART_NUM);
    return ESP_OK;
}

esp_err_t drv_max_m8q_read(gps_data_t *out_data)
{
    uint8_t buffer[GPS_BUF_SIZE];
    
    // Read raw data from UART ring buffer
    int len = uart_read_bytes(GPS_UART_NUM, buffer, GPS_BUF_SIZE - 1, pdMS_TO_TICKS(10));
    
    if (len > 0) {
        buffer[len] = '\0';
        // In a full driver implementation, we would pass 'buffer' to an NMEA parser (e.g., MinGPS or custom regex)
        // Parse sentences such as $GPGGA (altitude, sat count) and $GPRMC (latitude, longitude, speed, date)
        
        // TODO: Loop through NMEA sentences and parse tokens split by commas.
        
        // Mock parsing logs
        ESP_LOGD(TAG, "GPS UART raw data read: %d bytes", len);
    }

    // Populate mock GPS fix data for the skeleton demonstration
    out_data->latitude = 37.7749;       // Simulated San Francisco Lat
    out_data->longitude = -122.4194;    // Simulated San Francisco Lon
    out_data->altitude_m = 15.3;
    out_data->speed_kmh = 4.2;
    out_data->course_deg = 180.0;
    out_data->satellites = 8;
    out_data->has_fix = true;
    out_data->time.hour = 12;
    out_data->time.minute = 30;
    out_data->time.second = 45;

    return ESP_OK;
}

esp_err_t drv_max_m8q_sleep(void)
{
    ESP_LOGI(TAG, "Suspending GPS (MAX-M8Q) module...");
    // TODO: Send UBX proprietary binary packet (UBX-RXM-PMREQ) over UART to put GPS to sleep
    return ESP_OK;
}

esp_err_t drv_max_m8q_wakeup(void)
{
    ESP_LOGI(TAG, "Waking up GPS (MAX-M8Q) module...");
    // UBX chips wake up on any character input, send dummy character
    uint8_t dummy = 0xFF;
    uart_write_bytes(GPS_UART_NUM, (const char *)&dummy, 1);
    return ESP_OK;
}
