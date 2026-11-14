# ESP-IDF OpenThread C++

Wrapper OpenThread C++ 

## Requirements



## Build Instructions

Provide in your main directory a `Kconfig.projbuild with` this minimum contain:

```
menu "Project Settings"
    
    orsource "$IDF_PATH/examples/common_components/env_caps/$IDF_TARGET/Kconfig.env_caps"

    rsource "./Kconfig.main"
    rsource "./../components/thread/Kconfig.thread"
    rsource "./../components/mqttClient/Kconfig.mqtt"

    comment "Module Debugging"

    
endmenu

```

Main `CMakelists.txt` should look like this
```
set(SOURCES main.cpp)

idf_component_register(SRCS ${SOURCES}
                    INCLUDE_DIRS . 
                    REQUIRES mqttClient thread)
```

Provide a `sdkconfig.secret` should look like this

```Kconfig
# MQTT
CONFIG_MQTT_BROKER_URL="mqtt://[fde4:.....]"

CONFIG_MQTT_USERNAME="mqtt-user"
CONFIG_MQTT_PASSWORD="mqtt-password"

# OpenThread
CONFIG_TARGET_THREAD_TLV="0e080000000.............."
```

## Code example

```h
#pragma once

#include "mqttClient.h"
#include "thread.h"

// Main class used for testing only
class Main final
{

public:
    Main();

    void setupThread(void);

    void run(void);
    void setup(void);
    void onMqttConnection(bool connected, int res);
    void onMqttData(int msg_id, const std::string& topic, const std::string& data);
    void threadEvent(esp_openthread_event_t event_id);

private:
    MqttClient* _mqtt = nullptr;
    Thread*     _thread = nullptr;

}; // Main Class 

```

```cpp
#include "main.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_random.h" //todel

static const char *TAG = "MainApp";

Main App;


Main::Main()
{
    // Setting the log level for each module
    esp_log_level_set("Main_app", ESP_LOG_VERBOSE);  // Put verbose to check available stack
    esp_log_level_set("Wifi", ESP_LOG_VERBOSE);
    esp_log_level_set("MqttClient", ESP_LOG_VERBOSE);
}

void Main::setup(void)
{
    _thread = new Thread();
    _thread->registerHandler(&Main::threadEvent, this);
    _thread->init();
}

void  Main::threadEvent(esp_openthread_event_t event_id)
{
   if (event_id == OPENTHREAD_EVENT_GOT_IP6) {
        bool first = true;
        if(_mqtt)
            first = false;

        if(first)
            _mqtt = new MqttClient();
        
        
        _mqtt->stop();    
        _mqtt->start();

        if(first){
            _mqtt->registerConnectionHandler(&Main::onMqttConnection, this);
            _mqtt->registerDataHandler(&Main::onMqttData, this);
        }

    }
   
}

void Main::onMqttConnection(bool connected, int res)
{
    if (connected) {
        ESP_LOGI(TAG, "MQTT Connected. Session present: %d", res);
        _mqtt->subscribeTopic("akira/test1",0);
    } else {
        ESP_LOGW(TAG, "MQTT Disconnected. Error type: %d", res);
    }
}

void Main::onMqttData(int msg_id, const std::string& topic, const std::string& data)
{
    ESP_LOGI(TAG, "MQTT Data received. Message ID: %d, Topic: %s, Data: %s", msg_id, topic.c_str(), data.c_str());
}

void Main::run(void)
{
    int val = esp_random()%100;

    _mqtt->sendData("akira/test1", std::to_string(val));
    vTaskDelay(pdMS_TO_TICKS(2000)); // let time to reset watchdog
}


extern "C" void app_main(void)
{
    ESP_LOGI(TAG,"---------- Starting App ----------");
    App.setup();

    while (true)
    {
        App.run();
    }    

    //should not reach here
}

```

## OTA

Use a python script to send OTA (bin files). 

```json
import os
import socket
import struct
import sys

PORT = 3333
CHUNK_SIZE = 384
TIMEOUT = 1.0
MAX_RETRIES = 10

def print_progress_bar(sent, total, width=40):
    percent = sent / total if total > 0 else 1.0
    filled = int(width * percent)
    bar = "█" * filled + "-" * (width - filled)
    sys.stdout.write(f"\rSending OTA : |{bar}| {percent * 100:5.1f}% ({sent}/{total} bytes)")
    sys.stdout.flush()

def main():
    if len(sys.argv) < 3:
        print("Usage : python3 ota_script.py <DEVICE_IPV6> <FIRMWARE_BIN_PATH>")
        sys.exit(1)

    device_ipv6 = sys.argv[1]
    bin_path = sys.argv[2]

    if not os.path.isfile(bin_path):
        print(f"Error : File not found -> {bin_path}")
        sys.exit(1)

    total_size = os.path.getsize(bin_path)
    sent_bytes = 0
    seq = 0

    sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
    sock.settimeout(TIMEOUT)

    try:
        with open(bin_path, "rb") as f:
            while chunk := f.read(CHUNK_SIZE):
                packet = struct.pack("!I", seq) + chunk
                acked = False

                for attempt in range(MAX_RETRIES):
                    sock.sendto(packet, (device_ipv6, PORT))
                    try:
                        data, _ = sock.recvfrom(16)
                        if len(data) == 4 and struct.unpack("!I", data)[0] == seq:
                            acked = True
                            break
                    except socket.timeout:
                        continue

                if not acked:
                    print(f"\nFailed : no answer after {MAX_RETRIES} retries (packet {seq}).")
                    sys.exit(1)

                sent_bytes += len(chunk)
                seq += 1
                print_progress_bar(sent_bytes, total_size)

        sock.sendto(b"END", (device_ipv6, PORT))
        print("\nUpdate validated and written successfully !")

    finally:
        sock.close()

if __name__ == "__main__":
    main()
```
A VsCode task could be planned to run the python script:
Add a file `.vscode/tasks.json`

```json
{
    "version": "2.0.0",
    "inputs": [
        {
            "id": "threadDeviceIpv6",
            "type": "promptString",
            "description": "Enter Target OpenThread Device IPv6 Address",
            "default": "fdde:ad00:beef:0:..."
        }
    ],
    "tasks": [
        {
            "label": "Thread OTA Flash",
            "type": "shell",
            "command": "python3",
            "args": [
                "${workspaceFolder}/scripts/ota_script.py",
                "${input:threadDeviceIpv6}",
                "${workspaceFolder}/build/${workspaceFolderBasename}.bin"
            ],
            "group": {
                "kind": "build",
                "isDefault": true
            },
            "presentation": {
                "reveal": "always",
                "panel": "new"
            }
        }
    ]
}