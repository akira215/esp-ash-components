/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbCluster.h"

#include <esp_log.h> // TODEL development purpose

#include "zcl\esp_zigbee_zcl_ota.h"


class ZbOtaCluster : public ZbCluster
{

public:
    ZbOtaCluster( bool isClient = true,
                    uint32_t file_version = ESP_ZB_ZCL_OTA_UPGRADE_FILE_VERSION_DEF_VALUE,
                    uint16_t manufacturer = ESP_ZB_OTA_UPGRADE_MANUFACTURER_CODE_DEF_VALUE,
                    uint16_t image_type = ESP_ZB_OTA_UPGRADE_IMAGE_TYPE_DEF_VALUE,
                    uint16_t min_block_reque = ESP_ZB_OTA_UPGRADE_MIN_BLOCK_PERIOD_DEF_VALUE,
                    uint32_t file_offset = ESP_ZB_ZCL_OTA_UPGRADE_FILE_OFFSET_DEF_VALUE,
                    uint32_t downloaded_file_ver = ESP_ZB_ZCL_OTA_UPGRADE_DOWNLOADED_FILE_VERSION_DEF_VALUE,
                    //esp_zb_ieee_addr_t server_id = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
                    uint8_t image_upgrade_status = 1
                    ) 
    {
        esp_zb_ota_cluster_cfg_t cfg;
        cfg.ota_upgrade_file_version  = file_version;
        cfg.ota_upgrade_manufacturer = manufacturer;
        cfg.ota_upgrade_image_type = image_type;   
        cfg.ota_min_block_reque = min_block_reque;
        cfg.ota_upgrade_file_offset = file_offset;
        cfg.ota_upgrade_downloaded_file_ver = downloaded_file_ver;
        //cfg.ota_upgrade_server_id = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
        cfg.ota_image_upgrade_status = image_upgrade_status;

        for(int i=0;i<8;++i){
            cfg.ota_upgrade_server_id[i] = 0xff;
        }

        _attr_list = esp_zb_ota_cluster_create(&cfg);

        _init(ESP_ZB_ZCL_CLUSTER_ID_OTA_UPGRADE, isClient);
    }

    ///@brief Copy constructor
    ZbOtaCluster(const ZbOtaCluster& other)
    {
        esp_zb_ota_cluster_cfg_t cfg;
        cfg.ota_upgrade_file_version = *((uint32_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_OTA_UPGRADE_FILE_VERSION_ID)->data_p));
        cfg.ota_upgrade_manufacturer = *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_OTA_UPGRADE_MANUFACTURE_ID)->data_p));
        cfg.ota_upgrade_image_type = *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_OTA_UPGRADE_IMAGE_TYPE_ID)->data_p));   
        cfg.ota_min_block_reque = *((uint16_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_OTA_UPGRADE_MIN_BLOCK_REQUE_ID)->data_p));
        cfg.ota_upgrade_file_offset = *((uint32_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_OTA_UPGRADE_FILE_OFFSET_ID)->data_p));
        cfg.ota_upgrade_downloaded_file_ver = *((uint32_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_OTA_UPGRADE_DOWNLOADED_FILE_VERSION_ID)->data_p));
        cfg.ota_image_upgrade_status = *((uint8_t*)(other.getAttribute((uint16_t)
                            ESP_ZB_ZCL_ATTR_OTA_UPGRADE_IMAGE_STATUS_ID)->data_p));
        
        uint8_t* server_id = ((uint8_t*)(other.getAttribute((uint16_t)
                                ESP_ZB_ZCL_ATTR_OTA_UPGRADE_SERVER_ID)->data_p));
        
        for(int i=0;i<8;++i){
            cfg.ota_upgrade_server_id[i] = *(server_id+i);
        }
        
        _attr_list = esp_zb_ota_cluster_create(&cfg);

        _copyAttributes(other);

        _init(ESP_ZB_ZCL_CLUSTER_ID_OTA_UPGRADE, other.isClient());
    }

    virtual void addAttribute(uint16_t attr_id, void* value)
    {
        ESP_ERROR_CHECK(esp_zb_ota_cluster_add_attr(_attr_list, 
                            attr_id, value));
    }

    virtual void addToList(esp_zb_cluster_list_t* cluster_list)
    {
        ESP_ERROR_CHECK(esp_zb_cluster_list_add_ota_cluster(cluster_list, _attr_list, 
            isClient() ? ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE : ESP_ZB_ZCL_CLUSTER_SERVER_ROLE));
    }


private:
    /// @brief get the type and access according to ZCL specification
    /// @param attr_id the id of the attribute
    /// @return a struct with both data, to avoid 2 long switch testing
    attrType_t getAttrTypeAndAccess(uint16_t attr_id)
    {
        attrType_t attr = { ESP_ZB_ZCL_ATTR_TYPE_INVALID, 
                        ESP_ZB_ZCL_ATTR_ACCESS_INTERNAL };
        switch(attr_id)
        {
        default:
            ESP_LOGW("ZbOtaCluster",
                "Attribute desc not defined : attr id = %d", attr_id); // TODEL development purpose
            break;
        }
        return attr;
    }
   
};