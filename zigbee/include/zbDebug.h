/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/
#pragma once

#include <iostream>
#include <string>
#include "esp_zigbee_core.h"

namespace ZbDebug //optional
{

///////////////////////////////////// Clusters ///////////////////////////


void printAttr(esp_zb_attribute_list_t* attrList);

void printAttrList(esp_zb_attribute_list_t* attr_list);

void printCluster(esp_zb_zcl_cluster_t* cluster);

void printClusterList(esp_zb_cluster_list_t* clusterList);


///////////////////////////////////// Address ///////////////////////////

std::string addr2string(const esp_zb_ieee_addr_t addr );

///////////////////////////////////// Binding table ///////////////////////////

void print_binding_table_cb(const esp_zb_zdo_binding_table_info_t *record,
             void *user_ctx);

void print_binding_table_next(uint8_t index); 

void print_binding_table(const esp_zb_zdo_binding_table_record_t *record); 
} // namespace ZbDebug