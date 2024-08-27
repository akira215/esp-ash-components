# ESP-IDF Zigbee C++

Zigbee C++ 

## Requirements

- periodicTask
- esp-zboss-lib
- esp-zigbee-lib

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


