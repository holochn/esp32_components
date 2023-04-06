idf_component_register(
    SRCS "cppsrc/BMP180.cpp" 
    INCLUDE_DIRS "cppsrc"
    REQUIRES I2cMasterNode
)