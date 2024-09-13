/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "esp_zigbee_core.h"

/// @brief Class to interact to APS layer
class ZbApsData
{
/**
 * @brief Enumeration for APDS data indication cluster
 *
 */
typedef enum {
    ZB_APS_DATA_CLUSTER_BIND   =   0x0021

} zb_aps_data_cluster;

public:
    static ZbApsData* getInstance();

    //Singletons should not be cloneable.
    ZbApsData(ZbApsData &other) = delete;

    //Singletons should not be assignable.
    void operator=(const ZbApsData &) = delete;

    static bool zb_aps_data_indication_handler(esp_zb_apsde_data_ind_t msg);
    static void zb_aps_data_confirm_handler(esp_zb_apsde_data_confirm_t confirm);

private:
    ZbApsData();
};

