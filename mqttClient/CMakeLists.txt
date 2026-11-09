set(SOURCES mqttClient.cpp 
            )
            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include
                    REQUIRES espressif__mqtt)
