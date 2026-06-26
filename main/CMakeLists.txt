# CMakeLists.txt for the main component
idf_component_register(SRCS "main.c"
                       REQUIRES config drivers system ui nvs_flash)
