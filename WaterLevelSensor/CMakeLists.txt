idf_component_register(
    SRCS "cppsrc/WaterLevelSensor.cpp" 
    INCLUDE_DIRS "cppsrc"
    REQUIRES FrequencySensor nvs_flash
)