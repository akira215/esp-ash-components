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
    std::vector<unsigned char> _data;  // This is the vector of bytes storing data

    public:
        /// @brief Constructor
        mb_data(){};
        ~mb_data(){};

        /// @brief set size of the data buffer
        /// @param size: new size of data buffer
        void setSize(std::size_t size){
            _data.resize(size, 0);
        }
        
        /// @brief constructor (explicit to avoid int init)
        /// @param size: size of data buffer to be construct
        explicit mb_data(std::size_t size){
            setSize (size);
        }

        /// @brief get data size in bytes
        /// @return: size in bytes
        std::size_t getSize(){
            return _data.size();
        }


        /// @brief accessing the buffer to load value
        /// @return the buffer (unsafe to store pointer vector may be re allocate)
        void* buffer(){
            return (void*)(&_data[0]);
        }


        /////////////////////////////////////////////

        mb_data& operator = (const int16_t value){
            setValue(value);
            return *this;
        }


        ///////////////////////////////////////////////////
        /// @brief setValue, resize data if requires 
        /// @param value value to be set
        /// @param pos byte position to set the value
        void setValue(const int16_t value, const uint16_t pos = 0){ 
            //((unsigned char *)(&number))[n]
            if (getSize()<pos+2)
                setSize(pos+2);

            _data[pos] = ((unsigned char *)(&value))[0];
            _data[pos+1] = ((unsigned char *)(&value))[1];
        }


        ////////////////////////////////////////////

        operator int16_t() { return getValue();}

        ////////////////////////////////////////////

        int16_t getValue(){
            if (_data.size()==1)
                return (int16_t)(_data.at(0));

            if (_data.size()<1)
                return 0;
            
            return (int16_t)((unsigned char)(_data.at(0)) | (unsigned char)(_data.at(1)) << 8);
        }
        
        ////////////////////////////////////////////

        unsigned char getByte(uint16_t n){
            return _data.at(n);
        }


};