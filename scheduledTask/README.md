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

use std::bind to pass argument to callback if required

```cpp

ScheduledTask* task = new ScheduledTask(&ZbNode::joinNetwork, this, delay_ms);
```

## Changes

### New in version 0.0.0


## Credits


