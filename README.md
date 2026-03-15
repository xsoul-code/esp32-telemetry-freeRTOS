# ESP32 Telemetry on FreeRTOS
This project is a real-time telemetry acquisition and logging system for a PIR Motion Sensor on ESP32 hardware using FreeRTOS. Sensor data is acquired locally and published over MQTT to a broker via WiFi.

It has three modes of operation: Simulation of PIR Sensor state acquisition, GPIO Polling acquisition, and GPIO Interrupt (ISR) acquisition.
Those three modes are available in menuconfig and must be set before compiling the build.

## Overview
The project is based on ESP-IDF — an integrated development framework written in C/C++ with a build environment written in Python.
The project was made with a hands-on approach to C programming and embedded systems, with greater awareness of bare-metal microcontroller workings.

### Data flow
```
┌──────────────┐    Queue      ┌──────────────────┐
│  PIR Sensor  │──────────────▶│  datalogger_task │──▶ ESP_LOG (UART)
│  HC-SR501    │  datasensor_t │                  │──▶ MQTT publish
└──────────────┘               └──────────────────┘
       │
  GPIO21 / ISR / Polling
       │
┌──────────────┐    WiFi    ┌─────────────────┐
│    ESP32     │───────────▶│  MQTT Broker    │
│              │            │  (e.g. Mosquitto│
└──────────────┘            └─────────────────┘
```

### MQTT
Sensor data is published to topic `sensor/pir` in the following format:
```
captured on: <timestamp_ms> ms with state <0|1>
```
WiFi credentials and MQTT broker IP are configured via `pass.h` (see Setup).

## Hardware
This repository was written exclusively for ESP-IDF on the ESP32 family of microcontrollers.
The code was tested on real hardware: ESP32-DevKitC with a HC-SR501 PIR Motion Sensor Module (L Trigger mode).

### Wiring
```
┌─────────────────┐         ┌─────────────────────┐
│   HC-SR501      │         │    ESP32-DevKitC     │
│                 │         │                      │
│  VCC  ──────────┼─────────┼── 5V                 │
│  GND  ──────────┼─────────┼── GND                │
│  OUT  ──────────┼─────────┼── GPIO21             │
│                 │         │                      │
└─────────────────┘         └─────────────────────┘
```

## Setup

### WiFi and MQTT credentials
Copy `main/pass.h.example` to `main/pass.h` and fill in your credentials:
```c
#define WIFI_SSID      "your_ssid_here"
#define WIFI_PASS      "your_password_here"
#define MQTT_BROKER_IP "your_broker_ip_here"
```
> `pass.h` is listed in `.gitignore` and will never be committed.

### MQTT Broker
Any MQTT broker reachable on your local network works. Example setup with Mosquitto on Linux:
```bash
sudo apt install mosquitto mosquitto-clients
sudo systemctl start mosquitto
```
To monitor incoming data:
```bash
mosquitto_sub -h localhost -t "sensor/pir"
```

## How to build
```bash
idf.py menuconfig   # set PIR mode under PIR Configuration
idf.py build
idf.py flash monitor
```

## Requirements
- ESP-IDF v5.x
- ESP32 DevKitC or any ESP32-compatible device
- HC-SR501 PIR sensor or any digital input device
- MQTT broker (e.g. Mosquitto) accessible on local network

## References
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/)
- [HC-SR501 Datasheet](https://components101.com/sites/default/files/component_datasheet/HC%20SR501%20PIR%20Sensor%20Datasheet.pdf)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
- [ESP-IDF WiFi Station Example](https://github.com/espressif/esp-idf/tree/master/examples/wifi/getting_started/station)
- [ESP-IDF MQTT Documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/mqtt.html)