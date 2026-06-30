/*
  matter component for ESP32
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once


#include "freertos/FreeRTOS.h"
#include "freertos/task.h" // for task handle


static const char *MATTER_NODE_TAG = "MatterNode";


// Singleton class to manage matter node
class MatterNode
{

public:
  
    ~MatterNode();

    //Singletons should not be cloneable.
    MatterNode(MatterNode &other) = delete;

    //Singletons should not be assignable.
    void operator=(const MatterNode &) = delete;

    /// @brief Instanciante the obj or return the point to the unique obj
    static MatterNode* getInstance();


private:
    /// @brief Constructor is private (singleton)
    MatterNode();
 
}