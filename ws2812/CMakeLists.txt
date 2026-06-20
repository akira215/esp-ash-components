set(SOURCES ws2812.cpp
            )
      
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include
                    REQUIRES esp_driver_rmt)
