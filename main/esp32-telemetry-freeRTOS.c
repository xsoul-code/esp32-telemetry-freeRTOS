#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_random.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "connection.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "mqtt_client.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#define PIRIO GPIO_NUM_21
#define TAG "PIR_Sensor_System"

QueueHandle_t buffer;

typedef struct
{
	uint32_t timestamp;
	uint8_t PIR_sensor;
}datasensor_t;

void datastream_task(void *pvParameters)
{
	datasensor_t data1;
	
	//Set the mode in idf.py menuconfig before compiling
	#ifdef CONFIG_PIR_SIMULATION_MODE
		uint32_t dropped = 0;
		//Simulation of PIR Sensor behaviour
		
		ESP_LOGI(TAG, "is set to PIR_SIMULATION mode of operation in menuconfig"); //Changing mode using idf.py menuconfig
		
		while(1)
		{
			//Timestamp in ms
			data1.timestamp = esp_timer_get_time() / 1000;
			//Randomizing a boolean value of PIR
			data1.PIR_sensor = esp_random() % 2;

			BaseType_t result = xQueueSend(buffer, ( void * ) &data1, 0);
			if(result == errQUEUE_FULL)
			{
				dropped+=1;
				ESP_LOGW(TAG, " WARNING buffer full %d packet(s) dropped on %d ms reason: errQUEUE_FULL", dropped, data1.timestamp);
			}
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
		
	#else
		//GPIO Mode set in menuconfig -> PIR_Configuration
		ESP_LOGI(TAG, "is set to GPIO mode of operation in menuconfig"); //Changing mode using idf.py menuconfig
		uint32_t dropped = 0;
		while(1)
		{
			
			//Timestamp in ms
			data1.timestamp = esp_timer_get_time() / 1000;
			//Receiving a sensor data through GPIO
			data1.PIR_sensor = gpio_get_level(PIRIO);

			BaseType_t result = xQueueSend(buffer, ( void * ) &data1, 0);
			if(result == errQUEUE_FULL)
			{
				dropped+=1;
				ESP_LOGW(TAG, " error buffer full %d packet(s) dropped on %d ms reason: errQUEUE_FULL", dropped, data1.timestamp);
			}
			vTaskDelay(pdMS_TO_TICKS(1000));
		}
		
	#endif

}

void datalogger_task(void *pvParameters)
{
	datasensor_t data2;
	while(1)
	{
		if(xQueueReceive(buffer, ( void * ) &data2, pdMS_TO_TICKS(500)))
		{
			char payload[64];
			snprintf(payload, sizeof(payload), "captured on: %lu ms with state %d", data2.timestamp, data2.PIR_sensor);
			esp_mqtt_client_publish(mqtt_client, "sensor/pir", payload, 0, 1, 0);
			ESP_LOGI(TAG, "captured on: %d ms with state %d",data2.timestamp, data2.PIR_sensor);
		}
	}

}

void ISRdatastream_task(void *arg)
{
	datasensor_t isrdata;
	isrdata.timestamp = esp_timer_get_time() / 1000;
	isrdata.PIR_sensor = gpio_get_level(PIRIO);
	xQueueSendFromISR(buffer, ( void * ) &isrdata, NULL);
}


void app_main(void)
{
	// ESP WiFi Connection Init
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta(); // Function located in connection.c 

	// ESP MQTT Broker Init
	mqtt_init();

	buffer = xQueueCreate(2, sizeof(datasensor_t));
	//GPIO setup
	gpio_config_t pir_config ={
		.pin_bit_mask = (1ULL << PIRIO),
		.mode = GPIO_MODE_INPUT,
		.pull_down_en = GPIO_PULLDOWN_ENABLE,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.intr_type = GPIO_INTR_POSEDGE
	};

	gpio_config(&pir_config);
	xTaskCreate(datalogger_task, "data_logger", 2048, NULL, 5, NULL);

	#ifdef CONFIG_PIR_SIMULATION_MODE
		xTaskCreate(datastream_task, "data_stream", 2048, NULL, 5, NULL);
	#elif defined(CONFIG_PIR_POLLING)
		xTaskCreate(datastream_task, "data_stream", 2048, NULL, 5, NULL);
	#elif defined(CONFIG_PIR_ISR)
		gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
		gpio_isr_handler_add(PIRIO, ISRdatastream_task, NULL);
	#endif
}
