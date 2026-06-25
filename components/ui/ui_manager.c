/**
 * @file ui_manager.c
 * @brief User Interface Manager implementation for Flipper Black.
 */

#include "ui_manager.h"
#include "esp_log.h"
#include "event_bus.h"
#include "st7796s.h"
#include "screens.h"
#include "app_config.h"

static const char *TAG = "UI_MANAGER";

// Global UI State
static ui_screen_t current_screen = UI_SCREEN_MAIN_MENU;
static uint8_t battery_percent_ui = 100;
static bool has_gps_fix_ui = false;
static char nfc_uid_str[24] = "None";

// Mock LVGL structures for reference
typedef struct {
    uint16_t x;
    uint16_t y;
    bool is_pressed;
} mock_indev_data_t;

static mock_indev_data_t last_input = {0};

void ui_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing Graphical UI Library (LVGL Simulator)...");

    // 1. Initialize LVGL core
    // lv_init();

    // 2. Setup rendering buffers (usually double buffering in internal/external SRAM)
    ESP_LOGI(TAG, "Allocating display buffers (%d x %d pixels)...", LCD_WIDTH, LCD_HEIGHT / 10);
    // static lv_color_t buf1[LCD_WIDTH * 48];
    // static lv_color_t buf2[LCD_WIDTH * 48];
    // lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LCD_WIDTH * 48);

    // 3. Register LCD display driver callbacks
    ESP_LOGI(TAG, "Registering LCD display interface driver...");
    // static lv_disp_drv_t disp_drv;
    // lv_disp_drv_init(&disp_drv);
    // disp_drv.hor_res = LCD_WIDTH;
    // disp_drv.ver_res = LCD_HEIGHT;
    // disp_drv.flush_cb = my_disp_flush_cb; // points to drv_st7796s_flush
    // lv_disp_drv_register(&disp_drv);

    // 4. Register touch sensor callbacks
    ESP_LOGI(TAG, "Registering touch panel input driver...");
    // static lv_indev_drv_t indev_drv;
    // lv_indev_drv_init(&indev_drv);
    // indev_drv.type = LV_INDEV_TYPE_POINTER;
    // indev_drv.read_cb = my_touch_read_cb;
    // lv_indev_drv_register(&indev_drv);

    ESP_LOGI(TAG, "UI manager initialized. Displaying default Main Menu.");
    ui_show_screen(UI_SCREEN_MAIN_MENU);
}

// Function to transition between screens
void ui_show_screen(ui_screen_t screen)
{
    current_screen = screen;
    ESP_LOGI(TAG, "Switching Screen to: %s", ui_screen_name(screen));
    
    // Clear screen area and draw new components
    switch (screen) {
        case UI_SCREEN_MAIN_MENU:
            ESP_LOGI(TAG, "[UI Draw] Options: 1.Radio, 2.NFC, 3.GPS, 4.IR, 5.Settings");
            break;
        case UI_SCREEN_RADIO_SUBGHZ:
            ESP_LOGI(TAG, "[UI Draw] Sub-GHz Tools: Frequency: 433.92MHz | Modulation: OOK");
            break;
        case UI_SCREEN_RADIO_LORA:
            ESP_LOGI(TAG, "[UI Draw] LoRa Tools: SF7 | BW 125kHz | RSSI: N/A");
            break;
        case UI_SCREEN_NFC_TOOLS:
            ESP_LOGI(TAG, "[UI Draw] NFC Scanner. Place card close to back of device.");
            ESP_LOGI(TAG, "[UI Draw] Last UID Read: %s", nfc_uid_str);
            break;
        case UI_SCREEN_GPS_VIEW:
            ESP_LOGI(TAG, "[UI Draw] GPS Status: Fix=%s | Sats=%d", 
                     has_gps_fix_ui ? "YES" : "NO", has_gps_fix_ui ? 8 : 0);
            break;
        case UI_SCREEN_SETTINGS:
            ESP_LOGI(TAG, "[UI Draw] Settings: Brightness | Battery Stats | Calibration");
            break;
    }
}

// Read touch coordinates from input device and update internal structures
static void ui_poll_input_events(void)
{
    // TODO: Read hardware SPI from Touch Controller (XPT2046) using drv_touch_read()
    // Simulated periodic touch trigger
    static uint32_t ticks = 0;
    ticks++;
    if (ticks % 300 == 0) { // Every ~6 seconds
        last_input.x = 160;
        last_input.y = 240;
        last_input.is_pressed = true;
        
        ESP_LOGI(TAG, "Touch Input Event detected at X:%d, Y:%d", last_input.x, last_input.y);
        
        // Post event to Event Bus so the system registers the tap
        app_event_t event = {
            .type = EVENT_TYPE_UI_TOUCH
        };
        event.data.key.key_code = 1; // Simulated selection button
        event.data.key.is_pressed = true;
        event_bus_post(&event);
    }
}

// Render status bar elements
static void ui_draw_status_bar(void)
{
    // Prints mockup status bar
    ESP_LOGD(TAG, "[Status Bar] Bat: %d%% | GPS Fix: %s | Time: 12:30", 
             battery_percent_ui, has_gps_fix_ui ? "Fix" : "NoFix");
}

void ui_manager_update(void)
{
    // 1. Process local hardware inputs (Touch, Buttons)
    ui_poll_input_events();

    // 2. Consume events from the Event Bus queue to update screen state machines
    app_event_t event;
    // Check if there are any events on the bus without blocking (timeout = 0)
    while (event_bus_receive(&event, 0) == ESP_OK) {
        ESP_LOGI(TAG, "Received Event ID %d in UI Loop", event.type);
        
        switch (event.type) {
            case EVENT_TYPE_RF_SUBGHZ_RX:
                ESP_LOGI(TAG, "[UI Update] Packet received on sub-GHz: Length=%d, RSSI=%d", 
                         event.data.rf_rx.length, event.data.rf_rx.rssi);
                if (current_screen == UI_SCREEN_RADIO_SUBGHZ) {
                    // Update text block on Sub-GHz screen
                }
                break;

            case EVENT_TYPE_RF_LORA_RX:
                ESP_LOGI(TAG, "[UI Update] LoRa packet received: RSSI=%d", event.data.rf_rx.rssi);
                break;

            case EVENT_TYPE_NFC_TAG_DETECTED:
                snprintf(nfc_uid_str, sizeof(nfc_uid_str), "0x%02X%02X%02X%02X",
                         event.data.nfc_tag.uid[0], event.data.nfc_tag.uid[1],
                         event.data.nfc_tag.uid[2], event.data.nfc_tag.uid[3]);
                
                ESP_LOGI(TAG, "[UI Update] Tag Detected on UI: %s", nfc_uid_str);
                
                // If on NFC screen, play a buzzer tone and refresh the UID labels
                if (current_screen == UI_SCREEN_NFC_TOOLS) {
                    ui_show_screen(UI_SCREEN_NFC_TOOLS);
                }
                break;

            case EVENT_TYPE_GPS_UPDATE:
                has_gps_fix_ui = true;
                ESP_LOGI(TAG, "[UI Update] GPS Coordinates: Lat: %.4f, Lon: %.4f",
                         event.data.gps.latitude, event.data.gps.longitude);
                if (current_screen == UI_SCREEN_GPS_VIEW) {
                    ui_show_screen(UI_SCREEN_GPS_VIEW);
                }
                break;

            case EVENT_TYPE_BATTERY_CHARGING:
            case EVENT_TYPE_BATTERY_FULL:
            case EVENT_TYPE_BATTERY_LOW:
                battery_percent_ui = event.data.battery.percentage;
                break;

            case EVENT_TYPE_UI_TOUCH:
                // User clicked an item: cycle screens for the skeleton demo
                ESP_LOGI(TAG, "User touched screen. Navigating menu...");
                ui_screen_t next = (current_screen + 1) % UI_SCREEN_MAX;
                ui_show_screen(next);
                break;

            default:
                break;
        }
    }

    // 3. Draw static elements
    ui_draw_status_bar();
    
    // 4. Run LVGL time handler tick
    // lv_timer_handler();
}
