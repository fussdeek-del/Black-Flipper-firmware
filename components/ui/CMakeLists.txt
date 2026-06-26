# CMakeLists.txt for Flipper Black ui component
idf_component_register(SRCS "ui_manager.c"
                       INCLUDE_DIRS "include"
                       REQUIRES config system drivers)
