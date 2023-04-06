idf_component_register(
    SRCS "cppsrc/Wifi.cpp" 
    INCLUDE_DIRS "cppsrc"
    REQUIRES nvs_flash esp_event esp_netif esp_wifi
)
