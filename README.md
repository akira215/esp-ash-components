# ESP-IDF components by ASH

This repository gather ESP-IDF components. 

## Requirements


## Build Instructions

This repo is intented to be installed in your project directory as a submodule. In the root directory of your project, simply type:
```
git submodule add https://github.com/akira215/esp-ash-components.git components
```

## ESP component manager

Event if you don't require some component, ESP-IDF compnent manager will search all the idf_component.yml in the directory.
To avoid getting them in your project you must explicitely excludes them. In your root `CmakeLists.txt` place:


```
set(CMAKE_CXX_STANDARD 23)


set(EXTRA_COMPONENT_DIRS periodicTask cppgpio)

set(COMPONENTS   main)
set(EXCLUDE_COMPONENTS matter zigbee modbus mqttClient)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(testWifi)
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


