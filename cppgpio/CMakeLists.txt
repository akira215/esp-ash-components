set(SOURCES cppgpio_input.cpp
            cppgpio_output.cpp)

            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include
                    REQUIRES driver esp_event)
