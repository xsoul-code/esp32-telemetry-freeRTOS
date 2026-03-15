#pragma once
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "pass.h"

void wifi_init_sta(void);
extern esp_mqtt_client_handle_t mqtt_client;
void mqtt_init(void);

