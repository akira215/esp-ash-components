set(SOURCES periodicTask.cpp blinkTask.cpp)

            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include .
                    REQUIRES driver cppgpio)
