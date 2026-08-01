# Check Kconfig variable length
string(LENGTH "${CONFIG_ROOT_NODE_NAME}" STRING_LEN)
if(STRING_LEN GREATER 33)
    message(FATAL_ERROR "CONFIG_ROOT_NODE_NAME exceeds 33 allowed caracters for Matter stack! Edit sdkconfig by running menuconfig")
endif()


set(SOURCES matterNode.cpp
            matterEndpoint.cpp
            matterCluster.cpp
            matterAttribute.cpp)
            
idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS include
                    PRIV_INCLUDE_DIRS "../utils"
                    REQUIRES nvs_flash esp_matter eventLoop)

# ===  SILENCE THE WARNINGS OF ESP MATTER SDK===
# 1. Look up the exact path where the component manager installed esp_matter
idf_component_get_property(esp_matter_dir espressif__esp_matter COMPONENT_DIR)

if(esp_matter_dir)
    target_include_directories(${COMPONENT_LIB} SYSTEM PRIVATE 
        "${esp_matter_dir}/include"
        "${esp_matter_dir}/connectedhomeip/connectedhomeip/src"
        "${esp_matter_dir}/connectedhomeip/connectedhomeip/src/include"
    )
    message(STATUS "Applied SYSTEM includes to suppress warnings for: ${esp_matter_dir}")
else()
    message(WARNING "Could not locate espressif__esp_matter component directory to suppress warnings.")
endif()