# CMakeLists.txt for Flipper Black ui component
idf_component_register(SRCS "ui_manager.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES system drivers)
