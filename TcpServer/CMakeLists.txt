if(NOT TEST)
    idf_component_register(
        SRCS "cppsrc/TcpServer.cpp" 
        INCLUDE_DIRS "cppsrc"
    )
endif()