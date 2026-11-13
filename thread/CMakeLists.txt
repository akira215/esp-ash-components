set(SOURCES thread.cpp 
            )
            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include
                    REQUIRES openthread nvs_flash vfs app_update esp_netif lwip)
