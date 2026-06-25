# CMakeLists.txt for Flipper Black drivers component
idf_component_register(SRCS "cc1101.c"
                            "sx1262.c"
                            "pn532.c"
                            "max_m8q.c"
                            "st7796s.c"
                            "ir.c"
                            "w25q128.c"
                            "battery.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES driver esp_rom hal esp_timer)
