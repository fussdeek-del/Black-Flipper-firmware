/**
 * @file screens.h
 * @brief Layout screens enumeration and declarations.
 */

#pragma once

/**
 * @brief UI screen identifiers.
 */
typedef enum {
    UI_SCREEN_MAIN_MENU = 0,
    UI_SCREEN_RADIO_SUBGHZ,
    UI_SCREEN_RADIO_LORA,
    UI_SCREEN_NFC_TOOLS,
    UI_SCREEN_GPS_VIEW,
    UI_SCREEN_SETTINGS,
    UI_SCREEN_MAX
} ui_screen_t;

/**
 * @brief Switch the currently rendering active screen view.
 * 
 * @param screen Target screen enum identifier
 */
void ui_show_screen(ui_screen_t screen);

/**
 * @brief Helper function to get text label representation of screen name.
 * 
 * @param screen Target screen enum identifier
 * @return const char* String representation of screen
 */
static inline const char* ui_screen_name(ui_screen_t screen) {
    switch (screen) {
        case UI_SCREEN_MAIN_MENU:     return "Main Menu";
        case UI_SCREEN_RADIO_SUBGHZ:  return "Sub-GHz Tools";
        case UI_SCREEN_RADIO_LORA:     return "LoRa Tools";
        case UI_SCREEN_NFC_TOOLS:     return "NFC Scanner";
        case UI_SCREEN_GPS_VIEW:      return "GPS Coordinates";
        case UI_SCREEN_SETTINGS:      return "System Settings";
        default:                      return "Unknown Screen";
    }
}
