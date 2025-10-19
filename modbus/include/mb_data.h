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

    private:
        /// @brief Constructor for truncating
        mb_data( std::vector<unsigned char>::const_iterator first,  
                std::vector<unsigned char>::const_iterator last):
                _data(first, last){};

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
        /// @param pos byte position of the first byte to set the value
        void setValue(const int16_t value, const uint16_t pos = 0){ 
            //((unsigned char *)(&number))[n]
            if (getSize()<pos+2)
                setSize(pos+2);

            _data[pos] = ((unsigned char *)(&value))[0];
            _data[pos+1] = ((unsigned char *)(&value))[1];
        }


        ////////////////////////////////////////////

        operator uint16_t() const
        {   
            uint16_t res = 0;

            if (_data.size() > 0)
                ((unsigned char *)(&res))[0] = _data.at(0);
            
            if (_data.size() > 1)
                ((unsigned char *)(&res))[1] = _data.at(1);
            
            return res;
        }

        operator int16_t() const
        {   
            int16_t res = 0;

            if (_data.size() > 0)
                ((unsigned char *)(&res))[0] = _data.at(0);
            
            if (_data.size() > 1)
                ((unsigned char *)(&res))[1] = _data.at(1);
            
            return res;
        }

        operator uint32_t() const
        {   
            uint32_t res = 0;

            if (_data.size() > 0)
                ((unsigned char *)(&res))[2] = _data.at(0);
            
            if (_data.size() > 1)
                ((unsigned char *)(&res))[3] = _data.at(1);
            
            if (_data.size() > 2)
                ((unsigned char *)(&res))[0] = _data.at(2);
            
            if (_data.size() > 3)
                ((unsigned char *)(&res))[1] = _data.at(3);

            
            return res;
        }

        operator int32_t() const
        {   
            int32_t res = 0;

            if (_data.size() > 0)
                ((unsigned char *)(&res))[2] = _data.at(0);
            
            if (_data.size() > 1)
                ((unsigned char *)(&res))[3] = _data.at(1);
            
            if (_data.size() > 2)
                ((unsigned char *)(&res))[0] = _data.at(2);
            
             if (_data.size() > 3)
                ((unsigned char *)(&res))[1] = _data.at(3);

            
            return res;
        }

        operator uint64_t() const
        {   
            uint64_t res = 0;

            if (_data.size() > 0)
                ((unsigned char *)(&res))[6] = _data.at(0);
            
            if (_data.size() > 1)
                ((unsigned char *)(&res))[7] = _data.at(1);
            
            if (_data.size() > 2)
                ((unsigned char *)(&res))[4] = _data.at(2);
            
            if (_data.size() > 3)
                ((unsigned char *)(&res))[5] = _data.at(3);
            
            if (_data.size() > 4)
                ((unsigned char *)(&res))[2] = _data.at(4);
            
            if (_data.size() > 5)
                ((unsigned char *)(&res))[3] = _data.at(5);
            
            if (_data.size() > 6)
                ((unsigned char *)(&res))[0] = _data.at(6);
            
            if (_data.size() > 7)
                ((unsigned char *)(&res))[1] = _data.at(7);

            
            return res;
        }

        ////////////////////////////////////////////

        /// @brief to get a sub string of data
        /// @param pos byte position for data extraction
        /// @return return an mb_value starting by the position
        mb_data getDataFrom(const uint16_t pos = 0){
            if (pos > _data.size())
                return mb_data();
            
            std::vector<unsigned char>::const_iterator first = _data.begin() + pos;
            std::vector<unsigned char>::const_iterator last = _data.end();
            mb_data trunc(first, last);
            return trunc;
        }

        
        /// @brief getValue, 
        /// @param pos byte position of the first byte to get the value
        /// @return return value, truncated if not all bytes available 0 if data is too small
        int16_t getValue(const uint16_t pos = 0){
            if (_data.size() == pos + 1)
                return (int16_t)(_data.at(pos));

            if (_data.size() < pos + 1)
                return 0;
            
            return (int16_t)((unsigned char)(_data.at(pos)) | (unsigned char)(_data.at(pos+1)) << 8);
        }

        ////////////////////////////////////////////

        unsigned char getByte(const uint16_t n){
            return _data.at(n);
        }

};
