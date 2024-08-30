/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

//#include <vector>
#include <list>
#include <memory>
#include <cstring>
#include <string>
#include "esp_zigbee_core.h"


#include <iostream> // TODEL


class ZbAttribute
{
public:
    ZbAttribute(){};
    virtual ~ZbAttribute() {};
};


template <typename T>
class ZbData : public ZbAttribute {
    T _data;
public:
    ZbData(T data) {_data = data; }
    virtual ~ZbData() {};
    T* getDataPtr() {
        return &_data;
    }
};

template<> // specialization for char* used for short string 
class ZbData<const char*>: public ZbAttribute {
    char* _str;
public:
    ZbData(const char* cstr) { 
        // As per ZCL spec 1st byte is length. In case of long string use std::string type
        uint8_t length = (uint8_t)(*cstr) + 2; // one for \0 the other for the length
        _str = new char [length];
        std::strcpy(_str, cstr);
        }
    virtual ~ZbData() {};
    char* getDataPtr() {
        return _str;
    }
};
//TODO implement for long string with std::string (to differentiate the 2 bytes lentgh)


class ZbCluster
{
    esp_zb_zcl_cluster_t _cluster;
    std::list<std::unique_ptr<ZbAttribute>> _attrValues;
    std::list<esp_zb_attribute_list_t> _attrList;

public:
    /// @brief Constructor
    /// @param id Id of the endpoint
    ZbCluster(uint16_t id, bool isClient);
    ~ZbCluster();

    template <typename T>
    void addAttribute(uint16_t attr_id,
                    esp_zb_zcl_attr_type_t type, 
                    esp_zb_zcl_attr_access_t access,
                    const ZbData<T>& value)
        {
            // Store the attribute value
            
            ZbData<T>* data = new ZbData<T>(value);
            _attrValues.push_back(std::unique_ptr<ZbAttribute>(data));

            esp_zb_attribute_list_t newAttribute;

            newAttribute.cluster_id = _cluster.cluster_id;
            newAttribute.next = 0;
            newAttribute.attribute.id = attr_id;
            newAttribute.attribute.type = type;
            newAttribute.attribute.access = access; 
            newAttribute.attribute.manuf_code = 0xffff; // TODO check
            newAttribute.attribute.data_p = (data->getDataPtr());

            // Update next pointer
            esp_zb_attribute_list_t** n = &_attrList.back().next;
            _attrList.push_back(newAttribute);
            (*n) = &_attrList.back();


            // TODO check what is cluster attr_count;
            // Increment Cluster attr count
            // _cluster.attr_count += 1;
        }
    void getAttribute(uint16_t attr_id);

    esp_zb_zcl_cluster_t* getClusterStruct();

private:

        
};
