/*
  persistedValue
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#include <esp_log.h>


#include "freertos/FreeRTOS.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <string>
#include <cstdlib>


static const char *PERSISTED_TAG = "PersistedValue";

template<class T>
class PersistedValue
{
    T _value;
    std::string _key;
    nvs_handle_t _handle;

public:
    PersistedValue(const std::string& name, T value = 0, const std::string& namesp = std::string("nvs")):
                _value(value), _key(name) 
    {
        ESP_ERROR_CHECK(nvs_flash_init());
        ESP_LOGV(PERSISTED_TAG,"Opening nvs namespace %s", namesp.c_str());
        ESP_ERROR_CHECK(nvs_open(namesp.c_str(), NVS_READWRITE, &_handle));
        ESP_LOGV(PERSISTED_TAG,"Value %s created with handle", _key.c_str(), _handle);
        
        esp_err_t err = readValue(&_value);
        if (err!=ESP_OK)
            ESP_LOGW(PERSISTED_TAG,"Error during reading NVS - Error:%d ", err);
    }
    ~PersistedValue(){
        ESP_LOGD(PERSISTED_TAG,"Closing NVS for Value %s, NVS handle %d", 
                _key.c_str(), _handle);
        save();
        nvs_close(_handle);
    }

    void save (){
        // Read value on NVS to avoid writing same value
        T read;
        readValue(&read);
        if (_value != read){
            ESP_LOGD(PERSISTED_TAG,"Value %s differ with actual value, writing in NVS handle %d", 
                _key.c_str(), _handle);
            ESP_ERROR_CHECK(writeValue());
            ESP_ERROR_CHECK(nvs_commit(_handle));
        } else
            ESP_LOGI(PERSISTED_TAG,"Same value on NVS, nothing to write");
    }

    T getValue() { return _value;}
    operator T() { return _value;}

    PersistedValue<T> & operator = (const T & newValue){
        setValue(newValue);
        return *this;}

    void setValue(const T& newValue){
        _value = newValue;
    }

private:
    esp_err_t readValue(T* res){
        size_t len  = sizeof(T);
        ESP_LOGV(PERSISTED_TAG,"Reading value %s, handle %d", _key.c_str(), _handle);
        return nvs_get_blob(_handle, _key.c_str(), (void*)(res), &len);
    }
    esp_err_t writeValue(){
        size_t len  = sizeof(T);
        ESP_LOGV(PERSISTED_TAG,"Writing value %s, handle %d", _key.c_str(), _handle);
        return nvs_set_blob(_handle, _key.c_str(), (void*)(&_value), len);
    }

}; // PersistedValue Class

