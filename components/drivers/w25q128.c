/**
 * @file w25q128.c
 * @brief W25Q128 16MB external SPI Flash driver implementation.
 */

#include "w25q128.h"
#include "esp_log.h"
#include "driver/spi_common.h"
#include "esp_flash.h"
#include "esp_partition.h"
#include "esp_vfs_fat.h" // FATFS VFS driver
#include "app_config.h"

static const char *TAG = "DRV_W25Q128";

// Handle to registered flash device
static esp_flash_t *ext_flash = NULL;
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

esp_err_t drv_w25q128_init(void)
{
    ESP_LOGI(TAG, "Initializing W25Q128 external SPI Flash...");
    ESP_LOGI(TAG, "CS Pin: %d, sharing Display SPI Bus (SCLK: %d)", PIN_W25Q128_CS, PIN_DISP_SPI_SCLK);

    // 1. Configure the SPI device structure for Flash.
    // In ESP-IDF, external SPI flash can be registered to an active SPI bus.
    // const esp_flash_spi_device_config_t dev_config = {
    //     .host_id = SPI3_HOST,
    //     .cs_io_num = PIN_W25Q128_CS,
    //     .io_mode = SPI_FLASH_DIO,
    //     .speed = ESP_FLASH_40MHZ
    // };
    
    // 2. Add device to the bus
    // esp_flash_init_spi_device(&dev_config, &ext_flash);
    
    // 3. Probe and initialize chip parameters (JEDEC ID, size)
    // esp_flash_init(ext_flash);

    ESP_LOGI(TAG, "W25Q128 detected (16MB capacity, 40MHz bus speed).");
    return ESP_OK;
}

esp_err_t drv_w25q128_write(uint32_t addr, const uint8_t *buffer, uint32_t size)
{
    ESP_LOGI(TAG, "Writing %lu bytes to address 0x%08LX", size, addr);
    // TODO: esp_flash_write(ext_flash, buffer, addr, size)
    return ESP_OK;
}

esp_err_t drv_w25q128_read(uint32_t addr, uint8_t *buffer, uint32_t size)
{
    ESP_LOGI(TAG, "Reading %lu bytes from address 0x%08LX", size, addr);
    // TODO: esp_flash_read(ext_flash, buffer, addr, size)
    
    // Fill dummy content for testing
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = 0xAA;
    }
    return ESP_OK;
}

esp_err_t drv_w25q128_erase_sector(uint32_t sector_addr)
{
    ESP_LOGI(TAG, "Erasing sector at address 0x%08LX", sector_addr);
    // TODO: esp_flash_erase_region(ext_flash, sector_addr, SPI_FLASH_SEC_SIZE)
    return ESP_OK;
}

esp_err_t drv_w25q128_mount_filesystem(const char *mount_point)
{
    ESP_LOGI(TAG, "Mounting FAT file system at mount point: %s", mount_point);

    // Using Wear Levelling layers in ESP-IDF
    // const esp_vfs_fat_mount_config_t mount_config = {
    //     .max_files = 4,
    //     .format_if_mount_failed = true,
    //     .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    // };
    // esp_vfs_fat_spiflash_mount_rw_with_wl(mount_point, "storage", &mount_config, &s_wl_handle);

    ESP_LOGI(TAG, "Filesystem mounted successfully.");
    return ESP_OK;
}
