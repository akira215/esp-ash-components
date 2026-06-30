set(SOURCES matterNode.cpp)
            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include
                    REQUIRES nvs_flash )
#driver is required for zigbee stack