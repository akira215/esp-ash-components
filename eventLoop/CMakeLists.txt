set(SOURCES eventLoop.cpp)

            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include .
                    REQUIRES nvs_flash)
