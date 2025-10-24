# scheduledTask

scheduled a oneshot task using software timer

## Requirements

Only ESP-IDF framework is required

## Build Instructions

Add all the esp-ash-components as a git submodule of your project

## Current Status

Still work in progress, but is completely running. All bugs 
could be reported to try to improve it.

## Performance



## Caveats

### To be developped



## Usage and Examples

To pass argument to callback if required, it shall be after all arg list

```cpp

ScheduledTask* task = new ScheduledTask(&ZbNode::joinNetwork, this, delay_ms);

 // with arguments
ScheduledTask* testTask = new ScheduledTask(&AldesDriver::setFilterTimer, this, 
                                            10000, std::string("testFilters"), true, (uint16_t)10);

....

void AldesDriver::setFilterTimer(uint16_t remaining_days)
```

## Changes

### New in version 0.0.0


## Credits


