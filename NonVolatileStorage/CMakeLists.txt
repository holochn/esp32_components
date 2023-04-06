#if(NOT TEST)
    idf_component_register(
        SRCS "cppsrc/NonVolatileStorage.cpp" 
        INCLUDE_DIRS "cppsrc"
        REQUIRES nvs_flash
    )
#endif()
