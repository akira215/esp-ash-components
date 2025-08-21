set(SOURCES zbNode.cpp 
            zbEndpoint.cpp 
            zbCluster.cpp 
            zbDebug.cpp
            zbApsData.cpp)
            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include include/zbCluster include/zbClusterUtils
                    REQUIRES driver nvs_flash ieee802154 esp-zigbee-lib
                    nvs_flash partition_table esp_event
                    periodicTask eventLoop scheduledTask)
#driver is required for zigbee stack