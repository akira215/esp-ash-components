# Check Kconfig variable length
string(LENGTH "${CONFIG_ROOT_NODE_NAME}" STRING_LEN)
if(STRING_LEN GREATER 33)
    message(FATAL_ERROR "CONFIG_ROOT_NODE_NAME exceeds 33 allowed caracters for Matter stack! Edit sdkconfig by running menuconfig")
endif()


set(SOURCES matterNode.cpp
            matterEndpoint.cpp)
            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include
                    PRIV_INCLUDE_DIRS "../utils"
                    REQUIRES nvs_flash esp_matter)
#driver is required for zigbee stack