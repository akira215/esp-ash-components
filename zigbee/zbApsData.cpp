/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/
#include "zbApsData.h"

#include <iomanip>
#include <sstream>

#include <esp_log.h>

static const char *TAG = "ZB_APS";

ZbApsData* ZbApsData::getInstance()
{
    static ZbApsData instance;
    return &instance;
}

ZbApsData::ZbApsData()
{
    esp_zb_aps_data_indication_handler_register(zb_aps_data_indication_handler);
    esp_zb_aps_data_confirm_handler_register(zb_aps_data_confirm_handler);
}

// Static
bool ZbApsData::zb_aps_data_indication_handler(esp_zb_apsde_data_ind_t msg)
{
    ESP_LOGI("ZB_APS", "profile Id %d status %d Dst endp %d Cluster %d: %ld - Security %d", 
                         msg.profile_id, msg.status,  msg.dst_endpoint, msg.cluster_id, 
                         msg.asdu_length,  msg.security_status);

   
    // Page 106 zigbeealliance.orgdia


    switch(msg.cluster_id)
    {
        case ZB_APS_DATA_CLUSTER_BIND:
            ESP_LOGD(TAG, "Bind Request");
            //break;
        default:
            {
                // Print datas
                std::ostringstream output;
                char* curByte = (char*)( msg.asdu);
                output << std::hex;
                for(uint32_t i = 0; i< msg.asdu_length;i++)
                    output << std::setw(2) << std::setfill('0') << 
                            static_cast<unsigned>(*(curByte + i)) << " ";

                ESP_LOGI("ZB_APS", "data: %s", output.str().c_str());
            }
            break;
    }

    return false;
}

// Static
void ZbApsData::zb_aps_data_confirm_handler(esp_zb_apsde_data_confirm_t confirm)
{
    ESP_LOGI("ZB_APS", "Confirm status %d dst endp %d <- src endp %d, length %ld", confirm.status,
                        confirm.dst_endpoint,confirm.src_endpoint,confirm.asdu_length);
}