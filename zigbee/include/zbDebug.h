#pragma once

#include <iostream>
#include <string>
#include "esp_zigbee_core.h"

namespace ZbDebug //optional
{

inline void printAttr(esp_zb_attribute_list_t* attrList)
{
    std::cout << "clusterId | nextAddr | attrId| type | access |  manuf  |  dataAddr  |  data " << std::hex << std::endl 
        << +(attrList->cluster_id) << " | " 
        << attrList->next << " | " 
        << +(attrList->attribute.id) << " | " 
        << +(attrList->attribute.type) << " | " 
        << +(attrList->attribute.access) << " | " 
        << +(attrList->attribute.manuf_code) << " | " 
        << attrList->attribute.data_p << " | ";

    switch(attrList->attribute.type){
        case 0x20:
            std::cout << +(*((uint8_t*)(attrList->attribute.data_p)));
            break;
        case 0x21:
            std::cout << +(*((uint16_t*)(attrList->attribute.data_p)));
            break;
        case 0x23:
            std::cout << +(*((uint32_t*)(attrList->attribute.data_p)));
            break;
        case 0x30:
            std::cout << "E " << +(*((uint8_t*)(attrList->attribute.data_p)));
            break;
        case 0x42: // char*
            {
            uint8_t len = (*((uint8_t*)(attrList->attribute.data_p)));
            char* str = (char *)malloc((size_t)(len + 1));
            str = (char*)((attrList->attribute.data_p) + 1);
            str[len] = '\0';
            std::cout << str;
            }
            break;
        case 0x44: // long char*
            {
            uint16_t len = (*((uint16_t*)(attrList->attribute.data_p)));
            char* str = (char *)malloc((size_t)(len + 1));
            str = (char*)((attrList->attribute.data_p) + 2);
            str[len] = '\0';
            std::cout << str;
            }
            break;
        default:
            std::cout << "NotImpl";
            break;
    }
        std::cout << std::endl << "--------------------------------------------------------------------------"<< std::endl;
}

inline void printAttrList(esp_zb_attribute_list_t* attr_list)
{
    uint16_t counter = 0;
    while (attr_list){
        std::cout << "Cluster " << attr_list->cluster_id 
                << " - Attr " << counter 
                << " - @ " << attr_list << std::endl;
        ZbDebug::printAttr(attr_list);
        attr_list = attr_list->next;
        ++counter;
    }
   
}

inline void printCluster(esp_zb_zcl_cluster_t* cluster)
{
    std::cout << std::hex   << "============================== Cluster id " << (cluster->cluster_id) 
                            <<" ============================== "  << std::endl;
    std::cout << "attr count | role mask | manuf code" << std::endl;
    std::cout << unsigned(cluster->attr_count)  << " | " 
            << unsigned(cluster->role_mask) << " | "
            << unsigned(cluster->manuf_code) << " | "
            << std::endl;
    std::cout   << "============================= Attributes "
                <<"==============================" << std::endl;  

    printAttrList(cluster->attr_list);    
}

inline void printClusterList(esp_zb_cluster_list_t* clusterList)
{
    std::cout << std::hex << "================= Print Cluster List ======================= "  << std::endl;
    uint16_t counter = 0;
    while (clusterList){
        
        printCluster(&clusterList->cluster);
        clusterList = clusterList->next;
        ++counter;
        std::cout << std::endl;
    }
  
}

inline std::string addr2string(esp_zb_ieee_addr_t addr )
{

    std::string str;
    str.reserve(24);   // two digits per character + :

    static constexpr char hex[] = "0123456789ABCDEF";

    for (int i = 7; i !=-1 ; i--) {
       str.push_back(hex[(addr)[i] / 16]);
       str.push_back(hex[(addr)[i] % 16]);
       str.push_back(':');
    }

    return str;
    
  
}

}