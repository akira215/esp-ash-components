#include "persistedValue.h"

template <>
esp_err_t PersistedValue<int8_t>::readValue(int8_t* res) {
    return nvs_get_i8(_handle, _key.c_str(), res);
}

template <>
esp_err_t PersistedValue<uint8_t >::readValue(uint8_t * res) {
    return nvs_get_u8(_handle, _key.c_str(), res);
}

template <>
esp_err_t PersistedValue<int16_t>::readValue(int16_t* res) {
    return nvs_get_i16(_handle, _key.c_str(), res);
}

template <>
esp_err_t PersistedValue<uint16_t >::readValue(uint16_t * res) {
    return nvs_get_u16(_handle, _key.c_str(), res);
}

template <>
esp_err_t PersistedValue<int32_t>::readValue( int32_t* res) {
    return nvs_get_i32(_handle, _key.c_str(), res);
}

template <>
esp_err_t PersistedValue<uint32_t>::readValue(uint32_t* res) {
    return nvs_get_u32(_handle, _key.c_str(), res);
}

template <>
esp_err_t PersistedValue<int64_t>::readValue(int64_t* res) {
    return nvs_get_i64(_handle, _key.c_str(), res);
}

template <>
esp_err_t PersistedValue<uint64_t>::readValue(uint64_t* res) {
    return nvs_get_u64(_handle, _key.c_str(), res);
}

template <>
esp_err_t PersistedValue<std::string>::readValue(std::string* res) {
    size_t len;
    esp_err_t err;
    err = nvs_get_str(_handle, _key.c_str(), nullptr, &len);
    if(err!=ESP_OK)
        return err;
    
    char* data = (char*)malloc(len);
    err = nvs_get_str(_handle, _key.c_str(), data, &len);
    res->assign(data, len);
    return err;
}

// -========================= Write ==========================-

template <>
esp_err_t PersistedValue<int8_t>::writeValue() {
    return nvs_set_i8(_handle ,  _key.c_str(), _value);
}

template <>
esp_err_t PersistedValue<uint8_t>::writeValue() {
    return nvs_set_u8(_handle ,  _key.c_str(), _value);
}

template <>
esp_err_t PersistedValue<int16_t>::writeValue() {
    return nvs_set_i16(_handle ,  _key.c_str(), _value);
}

template <>
esp_err_t PersistedValue<uint16_t>::writeValue() {
    return nvs_set_u16(_handle ,  _key.c_str(), _value);
}

template <>
esp_err_t PersistedValue<int32_t>::writeValue() {
    return nvs_set_i32(_handle ,  _key.c_str(), _value);
}

template <>
esp_err_t PersistedValue<uint32_t>::writeValue() {
    return nvs_set_u32(_handle ,  _key.c_str(), _value);
}

template <>
esp_err_t PersistedValue<int64_t>::writeValue() {
    return nvs_set_i64(_handle ,  _key.c_str(), _value);
}

template <>
esp_err_t PersistedValue<uint64_t>::writeValue() {
    return nvs_set_u64(_handle ,  _key.c_str(), _value);
}

template <>
esp_err_t PersistedValue<std::string>::writeValue() {
    return nvs_set_str(_handle, _key.c_str(), _value.c_str());
}