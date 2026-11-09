/*
  Wifi
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/
#pragma once

#include "esp_event.h"

class Wifi
{
    bool _isConnected = false;

public:
    Wifi();
    bool isConnected() { return _isConnected; }

private:
    void wifi_init_sta(void);
    
    static void wifi_event_handler(void* arg, esp_event_base_t event_base, 
                                   int32_t event_id, void* event_data);


}; // Wifi Class