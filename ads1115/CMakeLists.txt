set(SOURCES ads1115.cpp)
            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include
                    REQUIRES cppi2c cppgpio)
