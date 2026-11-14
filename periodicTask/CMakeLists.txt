set(SOURCES periodicTask.cpp blinkTask.cpp buttonTask.cpp)
        
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include .
                    REQUIRES driver esp_driver_gptimer cppgpio esp_event)
