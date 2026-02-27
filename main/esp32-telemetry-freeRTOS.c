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
