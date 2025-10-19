# ESP-IDF Modbus C++

Modbus C++ 

## Requirements

- periodicTask
- espressif/esp-modbus

The 2 lasts will be installed by the `idf_component.yml` file located in the lib folder.



## Build Instructions

This repo is intented to be installed in your project directory as a submodule. In the root directory of your project, simply type:
```
git submodule add https://github.com/akira215/esp-ash-components.git components
```


Project Kconfig shall refer Kconfig.zb:
```
menu "Project Settings"
    
    orsource "$IDF_PATH/examples/common_components/env_caps/$IDF_TARGET/Kconfig.env_caps"

    comment "Module Debugging"

    rsource "./../components/zigbee/Kconfig.zb"
    
endmenu
```

if `CONFIG_ZB_LED` is set to -1 or not defined, no led will be used by the library.



## Code example

```
    ESP_LOGI(MODBUS_TAG, "Testing setValue 2117");
    mb_data test;
    test = 2117;    // Assignement shall be done in separate line
                    // COnstructor param is size not value !
                    
    ESP_LOGV(MODBUS_TAG, "size is %d", test.getSize());
    ESP_LOGV(MODBUS_TAG, "value is 0x %02x %02x", test.getByte(0), test.getByte(1));
    ESP_LOGV(MODBUS_TAG, "value in dec %d", (int16_t)test);


    mb_data gen_data = _mb_master->readRegisters(AldesModbus::MB_ALDES_ADDR,
                                            AldesModbus::REG_PRODUCT_CODE,
                                            6);
                                        
    if (gen_data.getSize() > 0)
        _product_code = gen_data;

    ESP_LOGD(ALDES_TAG, "Product Code : %d - %s", 
                    _product_code, aldesDeviceFromCode(_product_code));

    _serial_num = gen_data.getDataFrom(4);

    ESP_LOGD(ALDES_TAG, "Serial Number : %d ", _serial_num);

    mb_data firm_ver = _mb_master->readRegisters(AldesModbus::MB_ALDES_ADDR,
                                            AldesModbus::REG_SOFT_VERSION,
                                            1);
    if (firm_ver.getSize() > 0)
        _firm_ver = firm_ver;

    ESP_LOGD(ALDES_TAG, "Firmware version : %d", _firm_ver);
```

## Current Status

Still work in progress, but is completely running. All bugs 
could be reported to try to improve it.

## Performance



## Caveats

### To be developped



## Usage and Examples



## Changes

### New in version 0.0.0


## Credits


