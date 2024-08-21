set(SOURCES periodicTask.cpp blinkTask.cpp buttonTask.cpp)

            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include .
                    REQUIRES driver cppgpio esp_event)
