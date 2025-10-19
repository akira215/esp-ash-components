set(SOURCES modbusMaster.cpp 
            )
            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include
                    REQUIRES driver nvs_flash ieee802154 esp-modbus
                    nvs_flash partition_table esp_event
                    periodicTask eventLoop scheduledTask periodicSoftTask)
#driver is required for zigbee stack