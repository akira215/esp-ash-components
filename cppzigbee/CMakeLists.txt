set(SOURCES zbnode.cpp cppzb_ep.cpp cppzb_cluster.cpp)
            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include
                    REQUIRES driver nvs_flash)
#driver is required for zigbee stack