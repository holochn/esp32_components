idf_component_register(
    SRCS "cppsrc/FrequencySensor.cpp" 
    INCLUDE_DIRS "cppsrc"
    REQUIRES esp_adc_cal
)