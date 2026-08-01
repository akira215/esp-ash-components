/*
  matter
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include <esp_matter_attribute_utils.h>


class MatterValue : public esp_matter_attr_val_t
{
    bool _typeCreate = true;
public:
    /// @brief Default constructor: Initializes structure as an invalid/empty block
    /// @param typeCreate if set to true, will auto create the type otherwise 
    /// it will let the type as it was
    MatterValue(bool typeCreate = true) : _typeCreate(typeCreate) {
        this->type = ESP_MATTER_VAL_TYPE_INVALID;
        std::memset(&this->val, 0, sizeof(this->val));
    }

    // Copy Constructor from a raw Espressif C struct instance
    MatterValue(const esp_matter_attr_val_t& raw_struct) {
        this->type = raw_struct.type;
        this->val = raw_struct.val;
    }

    // Copy Constructor from a raw Espressif C struct instance
    MatterValue(const MatterValue& val) {
        this->type = val.type;
        this->val = val.val;
        this->_typeCreate = val._typeCreate;
    }

    // Copy overloading = operator
    MatterValue& operator=(const esp_matter_attr_val_t& raw_struct) {
        this->type = raw_struct.type;
        this->val = raw_struct.val;
        return *this;
    }

    // Copy overloading = operator
    MatterValue& operator=(const MatterValue& val) {
        this->type = val.type;
        this->val = val.val;
        this->_typeCreate = val._typeCreate;
        return *this;
    }

    // Helper to check if the underlying pointer is null or invalid
    bool isValid() const {
        return type != ESP_MATTER_VAL_TYPE_INVALID;
    }

    void setTypeCreate(bool typeCreate) { _typeCreate = typeCreate; }
    bool getTypeCreate() const { return _typeCreate; }


    // ------ Getters esp_matter_attr_val_t ------ //
    // 1. Unhandled types throw a clean compile-time error
    template <typename T>
    operator T() const {
        static_assert(sizeof(T) == 0, "Attempted to access an unsupported type from MatterValue!");
        return T{};
    }


    // 2. Conversion to bool
    operator bool() const {
        if ((this->type == ESP_MATTER_VAL_TYPE_BOOLEAN) || 
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_BOOLEAN))
            return this->val.b;
        return false;
    }

    // 3. Conversions to Unsigned Integers
    operator uint8_t() const {
        if ((this->type == ESP_MATTER_VAL_TYPE_UINT8) || 
            (this->type == ESP_MATTER_VAL_TYPE_ENUM8) ||
            (this->type == ESP_MATTER_VAL_TYPE_BITMAP8) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_UINT8) || 
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_ENUM8) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP8) )
            return this->val.u8;
        return 0;
    }
    operator uint16_t() const {
        if ((this->type == ESP_MATTER_VAL_TYPE_UINT16) ||
            (this->type == ESP_MATTER_VAL_TYPE_BITMAP16) ||
            (this->type == ESP_MATTER_VAL_TYPE_ENUM16) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_UINT16) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP16) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_ENUM16) )
            return this->val.u16;
        return 0;
    }
    operator uint32_t() const {
        if ((this->type == ESP_MATTER_VAL_TYPE_UINT32) ||
            (this->type == ESP_MATTER_VAL_TYPE_BITMAP32) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_UINT32) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_BITMAP32) )
            return this->val.u32;
        return 0;
    }
    operator uint64_t() const {
        if ((this->type == ESP_MATTER_VAL_TYPE_UINT64) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_UINT64))
            return this->val.u64;
        return 0;
    }

    // 4. Conversions to Signed Integers
    operator int8_t() const {
        if ((this->type == ESP_MATTER_VAL_TYPE_INT8) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_INT8) )
            return this->val.i8;
        return 0;
    }
    operator int16_t() const {
        if ((this->type == ESP_MATTER_VAL_TYPE_INT16) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_INT16) )
            return this->val.i16;
        return 0;
    }
    operator int32_t() const {
        if ((this->type == ESP_MATTER_VAL_TYPE_INTEGER) ||
            (this->type == ESP_MATTER_VAL_TYPE_INT32) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_INTEGER) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_INT32) )
            return this->val.i32;
        return 0;
    }
    operator int64_t() const {
        if ((this->type == ESP_MATTER_VAL_TYPE_INT64) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_INT64) )
            return this->val.i64;
        return 0;
    }
    // 5. Conversion to Floating Point
    operator float() const {
        if ((this->type == ESP_MATTER_VAL_TYPE_FLOAT) ||
            (this->type == ESP_MATTER_VAL_TYPE_NULLABLE_FLOAT) )
            return this->val.f;
        return 0.0f;
    }

    // 6. Conversion to std::string
    operator std::string() const {
        if ((this->type == ESP_MATTER_VAL_TYPE_CHAR_STRING) ||
            (this->type == ESP_MATTER_VAL_TYPE_OCTET_STRING) )
            return std::string(reinterpret_cast<char*>(this->val.a.b), this->val.a.s);
        return "";
    }

    // TODO Missing to implement
    // ESP_MATTER_VAL_TYPE_ARRAY 
    // ESP_MATTER_VAL_TYPE_LONG_CHAR_STRING
    // ESP_MATTER_VAL_TYPE_LONG_OCTET_STRING 


    // ------ Setters esp_matter_attr_val_t ------ //
    // Fallback for unhandled types to throw a readable compile error
    template <typename T>
    MatterValue& operator=(const T& unsupported_val) {
        static_assert(sizeof(T) == 0, "Attempted to assign an unsupported data type to MatterValue");
        return *this;
    }

    MatterValue& operator=(bool v) {
        if (_typeCreate)
            this->type = ESP_MATTER_VAL_TYPE_BOOLEAN;
        this->val.b = v;
        return *this;
    }

    // Unsigned types
    MatterValue& operator=(uint8_t v) {
        if (_typeCreate)
            this->type = ESP_MATTER_VAL_TYPE_UINT8;
        this->val.u8 = v;
        return *this;
    }

    MatterValue& operator=(uint16_t v) {
        if (_typeCreate)
            this->type = ESP_MATTER_VAL_TYPE_UINT16;
        this->val.u16 = v;
        return *this;
    }

    MatterValue& operator=(uint32_t v) {
        if (_typeCreate)
            this->type = ESP_MATTER_VAL_TYPE_UINT32;
        this->val.u32 = v;
        return *this;
    }

    MatterValue& operator=(uint64_t v) {
        if (_typeCreate)
            this->type = ESP_MATTER_VAL_TYPE_UINT64;
        this->val.u64 = v;
        return *this;
    }

    // Signed types
    MatterValue& operator=(int8_t v) {
        if (_typeCreate)
            this->type = ESP_MATTER_VAL_TYPE_INT8;
        this->val.i8 = v;
        return *this;
    }
   
    MatterValue& operator=(int16_t v) {
        if (_typeCreate)
            this->type = ESP_MATTER_VAL_TYPE_INT16;
        this->val.i16 = v;
        return *this;
    }

    MatterValue& operator=(int32_t v) {
        if (_typeCreate)
            this->type = ESP_MATTER_VAL_TYPE_INT32;
        this->val.i32 = v;
        return *this;
    }

    MatterValue& operator=(int64_t v) {
        if (_typeCreate)
            this->type = ESP_MATTER_VAL_TYPE_INT64;
        this->val.i64 = v;
        return *this;
    }

    MatterValue& operator=(float v) {
        if (_typeCreate)
            this->type = ESP_MATTER_VAL_TYPE_FLOAT;
        this->val.f = v;
        return *this;
    }

    // Handles standard C++ strings (uses raw attribute array pointers internally)
    MatterValue& operator=(const std::string& v) {
        this->type = ESP_MATTER_VAL_TYPE_CHAR_STRING;
        this->val.a.b = reinterpret_cast<uint8_t*>(const_cast<char*>(v.data()));
        this->val.a.s = static_cast<uint16_t>(v.length());   
        this->val.a.max = static_cast<uint16_t>(v.length()); 
        return *this;
    }


}; // MatterValue