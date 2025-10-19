/*
  modbus
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include <inttypes.h>
#include <vector>



class mb_data
{
    std::vector<uint8_t> _data;  // This is the vector of bytes storing data

    public:
        /// @brief Constructor
        mb_data();
        ~mb_data();


};