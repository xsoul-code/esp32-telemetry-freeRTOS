idf_component_register(SRCS "esp32-telemetry-freeRTOS.c" "connection.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi nvs_flash mqtt esp_netif esp_event driver)
