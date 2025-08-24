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

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
/*
#include <stdlib.h>
#include <sys/time.h>
#include "freertos/FreeRTOS.h"
*/

#include "scheduledTask.h" // to del


/// @brief Implementing time cluster Client, with automatic synchronization 
/// using coordinator. Client app shall call syncRTC once, then
/// the sync process will be trigger every month
class ZbTimeClusterClient : public ZbTimeCluster
{
    
    static constexpr std::chrono::system_clock::time_point TP_REF = 
                        std::chrono::sys_days(
                        std::chrono::year_month_day(std::chrono::year(2000), 
                                                    std::chrono::month(1), 
                                                    std::chrono::day(1)));
    uint8_t _endpt;
    uint16_t _srv_addr;

public:

    void processSync(clusterEvent_t event, std::vector<attribute_t> attrs){
        std::string strTZ;
        std::string strDSToffset;
        std::string strDSTStart;
        std::string strDSTEnd;
        int32_t offset = 0;
        std::time_t etime = std::chrono::system_clock::to_time_t(
                        std::chrono::system_clock::now());
        
        for (auto & el : attrs){
            uint16_t attrId = el.attrId;
            void* value = el.value;

            ESP_LOGD(ZCLUSTER_TAG, "Time cluster event type %x attribute %x", event, attrId);
   
            switch(attrId){
                case ESP_ZB_ZCL_ATTR_TIME_TIME_ID:{
                    uint32_t utc = *(static_cast<uint32_t*>(value));
                    std::chrono::system_clock::time_point tp = TP_REF;
                    tp += std::chrono::seconds(utc);
                    etime = std::chrono::system_clock::to_time_t(tp);
                    struct timeval epoch = {etime , 0};
                    int res = settimeofday((const timeval*)&epoch, 0);
                    if(res==0){
                        char time_buf[64];
                        struct tm tinfo;
                        gmtime_r(&etime, &tinfo);
                        strftime(time_buf, sizeof(time_buf), "%c", &tinfo); 
                        ESP_LOGI(ZCLUSTER_TAG, "The system date/time has been synchronized to UTC: %s", time_buf);
                    } else {
                        ESP_LOGW(ZCLUSTER_TAG, "Error setting date/time from coordinator source");
                    }
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_TIME_STATUS_ID:{
                    uint8_t status = *(static_cast<uint8_t*>(value));
                    ESP_LOGD(ZCLUSTER_TAG, "Coordinator TimeStatus %d",status);
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_TIME_ZONE_ID:{
                    offset = *(static_cast<int32_t*>(value));
                    uint32_t val = abs(offset);
                    strTZ = "UTC";
                    strTZ += offset > 0 ? "-" : "+"; // setenv specify reverse (+ west side)
                    strTZ += std::format("{:02}", ((uint8_t)(val / 3600))) + ":";
                    strTZ += std::format("{:02}", ((uint8_t)((val % 3600)/60)))+ ":";
                    strTZ += std::format("{:02}", ((uint8_t)((val % 3600)%60)));
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_DST_START_ID:{
                    uint32_t dst_start = *(static_cast<uint32_t*>(value));
                    /// time in second, shall be updated every year
                    std::chrono::system_clock::time_point tp = TP_REF;
                    tp += std::chrono::seconds(dst_start);
                    std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(tp)};
                    std::chrono::system_clock::time_point curY = std::chrono::sys_days(
                                        std::chrono::year_month_day(std::chrono::year(ymd.year()), 
                                                                    std::chrono::month(1), 
                                                                    std::chrono::day(0)))
                                        + std::chrono::hours(0) 
                                        + std::chrono::minutes(0) 
                                        + std::chrono::seconds(0);
                    auto nbDays = std::chrono::duration_cast<std::chrono::days>(tp - curY);
                    auto nbHours = std::chrono::duration_cast<std::chrono::hours>(tp - curY - nbDays);
                    strDSTStart = "," + std::to_string(nbDays.count())
                                + "/" + std::to_string(nbHours.count());
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_DST_END_ID:{
                    uint32_t dst_end = *(static_cast<uint32_t*>(value));
                    /// time in second, shall be updated every year
                    std::chrono::system_clock::time_point tp = TP_REF;
                    tp += std::chrono::seconds(dst_end);
                    std::chrono::year_month_day ymd{std::chrono::floor<std::chrono::days>(tp)};
                    std::chrono::system_clock::time_point curY = std::chrono::sys_days(
                                        std::chrono::year_month_day(std::chrono::year(ymd.year()), 
                                                                    std::chrono::month(1), 
                                                                    std::chrono::day(0)))
                                        + std::chrono::hours(0) 
                                        + std::chrono::minutes(0) 
                                        + std::chrono::seconds(0);
                    auto nbDays = std::chrono::duration_cast<std::chrono::days>(tp - curY);
                    auto nbHours = std::chrono::duration_cast<std::chrono::hours>(tp - curY - nbDays);
                    strDSTEnd = "," + std::to_string(nbDays.count())
                              + "/" + std::to_string(nbHours.count());
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_DST_SHIFT_ID:{
                    int32_t dst_shift = *(static_cast<int32_t*>(value));
                    strDSToffset = "UTC"; 
                    strDSToffset += dst_shift > 0 ? "-" : "+"; // setenv specify reverse (+ west side)
                    strDSToffset += std::to_string( (uint8_t)((dst_shift + offset) / 3600));
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_STANDARD_TIME_ID:{
                    uint32_t* stdTime = static_cast<uint32_t*>(value);
                    ESP_LOGD(ZCLUSTER_TAG, "standard is %ld", *stdTime);
                    break;}
                case ESP_ZB_ZCL_ATTR_TIME_LOCAL_TIME_ID:{
                    uint32_t localTime = *(static_cast<uint32_t*>(value));
                    ESP_LOGD(ZCLUSTER_TAG, "localTime is %ld", localTime);
                    break;}
            } // case
        } // for

        std::string tz = strTZ + strDSToffset + strDSTStart + strDSTEnd;
        setenv("TZ",tz.c_str(),1); // You must include '0' after first designator e.g. GMT0GMT-1, ',1' is true or ON);
        tzset();
        ESP_LOGI(ZCLUSTER_TAG, "TZ env variable set to : %s", tz.c_str());

        /* C++ style
        auto n = std::chrono::system_clock::now();
        auto tt = std::chrono::system_clock::to_time_t(n);
        std::cout << "Current system Time " << std::ctime(&tt)<< std::endl;
        */
        time_t now;
        char strftime_buf[64];
        struct tm timeinfo;

        time(&now);
        localtime_r(&now, &timeinfo);
        strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo); 
        ESP_LOGI(ZCLUSTER_TAG, "Sync finisihed - the Current Local date/time is: %s", strftime_buf);

        //Trigger a new clock sync in one month
        ScheduledTask* task = new ScheduledTask(&ZbTimeClusterClient::syncRTC, this, 2635200000);
    };

public:
    ZbTimeClusterClient( uint8_t dst_endpoint = 1, 
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

        registerEventHandler(&ZbTimeClusterClient::processSync, this);
    };


    ///@brief Send request to coord to sync the local internal clock
    /// if enough enough infos clock will be sync after coord response
    void syncRTC(){
        uint16_t arr[] = {ESP_ZB_ZCL_ATTR_TIME_TIME_ID,
                            ESP_ZB_ZCL_ATTR_TIME_TIME_STATUS_ID,
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