
# ESP32 Telemetry on freeRTOS 

This project is a basic implementation of telemetric acquisition and logging of PIR Motion Sensor state on ESP32 hardware using FreeRTOS.
It has three modes of operation which are: Simulation of PIR Sensor state acquisition, GPIO Polling acquisition of state, GPIO interrupt(ISR) acquisition.
Those three modes are available in menuconfig and must be set before compiling the build.

## Overview

Whole project is based on ESP-IDF which is an integrated development framework written in C/C++ language and build environment written in Python.
Project was made with idea in mind to write a program for acquisition a typical digital signal with more hands-on approach with C programming language
and greater awareness of "bare metal" workings of microcontroller and other embedded systems alike.

The idea of data flow in system:

┌──────────────┐    Queue    ┌──────────────────┐
│  PIR Sensor  │────────────▶│  datalogger_task │──▶ ESP_LOG
│  HC-SR501    │  datasensor │                  │
└──────────────┘    _t       └──────────────────┘
       │
  GPIO21 / ISR / Polling
       │
┌──────────────┐
│    ESP32     │
└──────────────┘


## Hardware

This repository was written exclusively for ESP-IDF environment for programming and prototyping on ESP32 family of embedded systems/microcontrollers.
The code was tested on real hardware which was ESP32-DevKitC and digital signal source was chosen to be a PIR Motion Sensor HC SR501 Module with L Trigger
mode set on the sensor itself.

Connection of PIR Sensor to ESP32 Hardware:

┌─────────────────┐         ┌─────────────────────┐
│   HC-SR501      │         │    ESP32-DevKitC     │
│                 │         │                      │
│  VCC  ──────────┼─────────┼── 5V                 │
│  GND  ──────────┼─────────┼── GND                │
│  OUT  ──────────┼─────────┼── GPIO21             │
│                 │         │                      │
└─────────────────┘         └─────────────────────┘

## How to build

Building a repository needs ESP-IDF environment and it is required to set the mode of operation with menuconfig command under PIR Configuration.
More options is available through disabling the first option which is "Simulate PIR sensor". 

```bash
idf.py menuconfig
idf.py build
idf.py flash monitor
```

## Requirements
- ESP-IDF v5.x
- ESP32 DevKitC / or any other ESP32 compatible device see ESP32 documentation for more info.
- HC-SR501 PIR sensor / or any other digital input device 

## References
- [ESP-IDF Documentation](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/)
- [HC-SR501 Datasheet](https://components101.com/sites/default/files/component_datasheet/HC%20SR501%20PIR%20Sensor%20Datasheet.pdf)
- [FreeRTOS Documentation](https://www.freertos.org/Documentation/RTOS_book.html)
