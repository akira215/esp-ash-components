set(SOURCES zbNode.cpp zbEndpoint.cpp cppzb_cluster.cpp)
            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include
                    REQUIRES driver nvs_flash esp-zigbee-lib periodicTask)
#driver is required for zigbee stack