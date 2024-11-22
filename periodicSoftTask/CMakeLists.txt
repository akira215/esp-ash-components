set(SOURCES periodicSoftTask.cpp)
        
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include .
                    REQUIRES esp_timer)
