if(NOT TEST)
    idf_component_register(
        SRCS "cppsrc/HttpMessage.cpp" 
        INCLUDE_DIRS "cppsrc"
    )
endif()
