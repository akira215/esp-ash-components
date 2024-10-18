/*
  cppzigbee
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include "zbTimeCluster.h"

#include <chrono>
#include <format>
#include <string>
#include <cmath>

static const char *TAGT = "Main_app";

/// @brief Implementing time cluster Client, with automatic synchronisation using coordinator
class ZbTimeUtil : public ZbTimeCluster
{
    uint8_t _endpt;
    uint16_t _srv_addr;

public:

    void processSync(clusterEvent_t event, std::vector<attribute_t> attrs){
        
        for (auto & el : attrs){
            uint16_t attrId = el.attrId;
            void* value = el.value;

            ESP_LOGI(TAGT, "Time cluster event type %x attribute %x", event, attrId);
   
            switch(attrId){
                case ESP_ZB_ZCL_ATTR_TIME_TIME_ID:{
                    uint32_t* utc = static_cast<uint32_t*>(value);
                    ESP_LOGW(TAGT, "UTC is  %ld", (*utc));

                    //std::chrono::duration<uint32_t,std::chrono::seconds> duration(*utc);
                    std::chrono::system_clock::time_point tp = std::chrono::sys_days(
                        std::chrono::year_month_day(std::chrono::year(2000), std::chrono::month(1), std::chrono::day(1)))
                    + std::chrono::hours(0) + std::chrono::minutes(0) + std::chrono::seconds(0);
                    tp += std::chrono::seconds(*utc);
                    std::time_t etime = std::chrono::system_clock::to_time_t(tp);
                    std::cout << "Time " << std::ctime(&etime)<< std::endl;
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_TIME_ZONE_ID:{
                    int32_t* offset = static_cast<int32_t*>(value);
                    std::cout << "Time Zone " << (*offset)<< std::endl;
                    int8_t sign =  ((*offset) > 0) - ((*offset) < 0);
                    uint32_t val = abs(*offset);
                    char s = sign < 0 ? '-' : '+';
                    std::string strTZ = "UTC";
                    strTZ += sign < 0 ? "-" : "+";
                    strTZ += std::format("{:02}", (uint8_t)(val / 3600)) + ":";
                    strTZ += std::format("{:02}", (uint8_t)((val % 3600)/60))+ ":";
                    strTZ += std::format("{:02}", (uint8_t)((val % 3600)%60));
                    ESP_LOGI(TAGT, "Offset is %s", strTZ.c_str());
                    setenv("TZ", "UTC,M3.5.0/01,M10.5.0/02",1); // You must include '0' after first designator e.g. GMT0GMT-1, ',1' is true or ON);
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_DST_START_ID:{
                    uint32_t* dst_start = static_cast<uint32_t*>(value);
                    ESP_LOGI(TAGT, "dst_start is %ld", *dst_start);
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_DST_END_ID:{
                    uint32_t* dst_end = static_cast<uint32_t*>(value);
                    ESP_LOGI(TAGT, "dst_end is %ld", *dst_end);
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_DST_SHIFT_ID:{
                    int32_t* dst_shift = static_cast<int32_t*>(value);
                    ESP_LOGI(TAGT, "dst_end is %ld", *dst_shift);
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_STANDARD_TIME_ID:{
                    uint32_t* stdTime = static_cast<uint32_t*>(value);
                    ESP_LOGI(TAGT, "standard is %ld", *stdTime);
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_LOCAL_TIME_ID:{
                    uint32_t* localTime = static_cast<uint32_t*>(value);
                    ESP_LOGI(TAGT, "localTime is %ld", *localTime);
                    break;}
            } // case
        } // for
    };

public:
    ZbTimeUtil( uint8_t dst_endpoint = 1, 
                uint16_t short_addr = 0x0000 ) : ZbTimeCluster(true)
    {
        _endpt = dst_endpoint;
        _srv_addr = short_addr;

        int32_t timeZoneDefault     = ESP_ZB_ZCL_TIME_TIME_ZONE_DEFAULT_VALUE;
        uint32_t dstStartDefault    = ESP_ZB_ZCL_TIME_DST_START_DEFAULT_VALUE;
        uint32_t dstEndDefault      = ESP_ZB_ZCL_TIME_DST_END_DEFAULT_VALUE;
        int32_t dstShiftDefault     = ESP_ZB_ZCL_TIME_DST_SHIFT_DEFAULT_VALUE;
        uint32_t stdTimeDefault     = ESP_ZB_ZCL_TIME_STANDARD_TIME_DEFAULT_VALUE;
        uint32_t localTimeDefault   = ESP_ZB_ZCL_TIME_LOCAL_TIME_DEFAULT_VALUE;
        uint32_t lastSetDefault     = ESP_ZB_ZCL_TIME_LAST_SET_TIME_DEFAULT_VALUE;
        uint32_t validUntilDefault  = ESP_ZB_ZCL_TIME_VALID_UNTIL_TIME_DEFAULT_VALUE;

        addAttribute(ESP_ZB_ZCL_ATTR_TIME_TIME_ZONE_ID, &timeZoneDefault);
        addAttribute(ESP_ZB_ZCL_ATTR_TIME_DST_START_ID, &dstStartDefault);
        addAttribute(ESP_ZB_ZCL_ATTR_TIME_DST_END_ID, &dstEndDefault);
        addAttribute(ESP_ZB_ZCL_ATTR_TIME_DST_SHIFT_ID, &dstShiftDefault);
        addAttribute(ESP_ZB_ZCL_ATTR_TIME_STANDARD_TIME_ID, &stdTimeDefault);
        addAttribute(ESP_ZB_ZCL_ATTR_TIME_LOCAL_TIME_ID, &localTimeDefault);
        addAttribute(ESP_ZB_ZCL_ATTR_TIME_LAST_SET_TIME_ID, &lastSetDefault);
        addAttribute(ESP_ZB_ZCL_ATTR_TIME_VALID_UNTIL_TIME_ID, &validUntilDefault);

        registerEventHandler(&ZbTimeUtil::processSync, this);
    };

    void syncRTC(){
        uint16_t arr[] = {ESP_ZB_ZCL_ATTR_TIME_TIME_ID,
                            ESP_ZB_ZCL_ATTR_TIME_TIME_ZONE_ID,
                            ESP_ZB_ZCL_ATTR_TIME_DST_START_ID,
                            ESP_ZB_ZCL_ATTR_TIME_DST_END_ID,
                            ESP_ZB_ZCL_ATTR_TIME_DST_SHIFT_ID,
                            ESP_ZB_ZCL_ATTR_TIME_STANDARD_TIME_ID,
                            ESP_ZB_ZCL_ATTR_TIME_LOCAL_TIME_ID
                            };
                            
        readAttribute(std::span(arr), _endpt, _srv_addr);

    };
   
};