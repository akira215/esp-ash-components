/*
  matter component for ESP32
  Repository: https://github.com/akira215/esp-ash-components
  License: GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007
  Author: Akira Shimahara
*/

#pragma once

#include <vector>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h" // Required for Semaphore/Mutex APIs
#include "esp_log.h"

/// @brief clas to implement fast and memory efficient ordered map
/// 

template <typename T> 
class MatterMap 
{


    struct map_t {
        uint32_t id;
        T value;

        // Comparator needed for binary searching by address
        bool operator<(const map_t & other) const {
            return id< other.id;
        }
    };

    std::vector<map_t> _map;
    mutable SemaphoreHandle_t _mutex; // Mutable allows locking inside 'const' methods

     // 1. Declare the static variable ONCE here
    static const T* get_default_address() {
        static const T default_value = T();
        return &default_value;
    }

public:
    MatterMap() { _mutex = xSemaphoreCreateMutex(); }
    ~MatterMap() { if (_mutex != nullptr) vSemaphoreDelete(_mutex);   }

    // Expose manual locking for safe raw reference/pointer manipulation
    void lock() const { xSemaphoreTake(_mutex, portMAX_DELAY);  }
    void unlock() const {   xSemaphoreGive(_mutex); }

    // Pre-allocate memory to completely avoid runtime heap allocation
    void reserve(size_t capacity) { _map.reserve(capacity); }

    // Read/Write operator
    T& operator[](uint32_t id) {
        map_t target{id, T() };
        auto it = std::lower_bound(_map.begin(), _map.end(), target);

        if (it == _map.end() || it->id != id) {
            // The id does not exist, inderting it ordered
            it = _map.insert(it, map_t{id, T() });
        }

        return it->value;
    }

    // Const read operator
    const T& operator[](uint32_t id) const {
        map_t target{id, T()};
        auto it = std::lower_bound(_map.begin(), _map.end(), target);

        if (it == _map.end() || it->id != id) {
            ESP_LOGE("MatterMap", "Fatal: Id %lu missing from const map - return default value", (unsigned long)id);
            return *get_default_address();
        }

        return it->value;
    }

    /**
     * Efficiently updates or inserts a consecutive block of registers.
     * @param start_aid The address of the first register in the block
     * @param values Pointer to the array of incoming consecutive values
     * @param count Number of registers in the block
     */
    void insert_block(uint32_t start_id, const T* values, size_t count) {
        if (count == 0 || values == nullptr) return;

        // Ensure vector has enough total capacity to prevent hidden reallocations
        if (_map.size() + count > _map.capacity()) {
            _map.reserve(_map.size() + count + 10);
        }

        // 1. Find where the block should start in our sorted vector
        map_t target{start_id, T()};
        auto it = std::lower_bound(_map.begin(), _map.end(), target);

        size_t value_idx = 0;

        // 2. Overwrite existing registers that match the incoming block
        while (it != _map.end() && value_idx < count && it->id == (start_id + value_idx)) {
            it->value = values[value_idx];
            static_cast<void>(++it);
            value_idx++;
        }

        // If the entire block updated existing elements, we are done
        if (value_idx >= count) return;

        // 3. For remaining new items, check if they fall into a clean gaps or the end
        // Prepare a temporary batch array to insert everything remaining in one go
        std::vector<map_t> new_entries;
        new_entries.reserve(count - value_idx);

        for (size_t i = value_idx; i < count; ++i) {
            new_entries.push_back(map_t{static_cast<uint32_t>(start_id + i), values[i] });
        }

        // 4. Perform a single batch insertion, which moves memory down exactly ONCE
        _map.insert(it, new_entries.begin(), new_entries.end());

    }


    // Standard C++ exception-throwing at() variant (Thread-safe)
    T at(uint32_t id) const {
        lock();
        
        map_t target{id, T()};
        auto it = std::lower_bound(_map.begin(), _map.end(), target);

        if (it == _map.end() || it->id != id) {
            unlock(); // CRITICAL: Must unlock before throwing to prevent deadlocks!
            ESP_LOGE("MatterMap", "Fatal: Id %lu missing from at() - abort", (unsigned long)id);
            return *get_default_address();
        }

        T value_copy = it->value;
        unlock();
        return value_copy;
    }

    // return a pointer, so nullptr could be tested
    const T* get(uint32_t id) const {
        map_t target{id, T()};
        auto it = std::lower_bound(_map.begin(), _map.end(), target);

        if (it == _map.end() || it->id != id) {
            return nullptr; // No exception, no crash, explicitly signals failure
        }
        return &(it->value);
    }

    bool contains(uint32_t id) const {
        map_t target{id, T()};
        
        auto it = std::lower_bound(_map.begin(), _map.end(), target);

        return (it != _map.end() && it->id == id);    
    }

    size_t size() const {   return _map.size(); }

    void clear() { _map.clear();  } // Destroys all elements, size drops to 0

};