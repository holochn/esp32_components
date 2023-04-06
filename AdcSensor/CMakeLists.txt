idf_component_register(
    SRCS "cppsrc/AdcSensor.cpp" 
    INCLUDE_DIRS "cppsrc"
    REQUIRES esp_adc_cal nvs_flash
)