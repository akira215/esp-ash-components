# blinkTask value

Blinking task

## Requirements

Only ESP-IDF framework is required
cppgpio required

## Build Instructions

Add all the esp-ash-components as a git submodule of your project

## Current Status

Still work in progress, but is completely running. All bugs 
could be reported to try to improve it.

## Performance



## Caveats

### To be developped



## Usage and Examples


```cpp

void Main::setup(void)
{

    BlinkTask* ledBlinking = new BlinkTask(GPIO_NUM_23);
    vTaskDelay(pdMS_TO_TICKS(5000));
    
    ledBlinking->setDelay(FAST_BLINK);
    delete ledBlinking;

    BlinkTask* fastBlinking= new BlinkTask(GPIO_NUM_23,NORMAL_BLINK);

}
```

## Changes

### New in version 0.0.0


## Credits


