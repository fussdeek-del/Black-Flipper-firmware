# CMakeLists.txt for Flipper Black system component
idf_component_register(SRCS "task_manager.c" "event_bus.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES drivers)
