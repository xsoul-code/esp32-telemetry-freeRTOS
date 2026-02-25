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
	while(1)
	{
		//Timestamp in ms
		data1.timestamp = esp_timer_get_time() / 1000;
		//Randomizing a boolean value of PIR for test purposes
		data1.PIR_sensor = esp_random() % 2;
		xQueueSend(buffer, ( void * ) &data1, portMAX_DELAY);
		vTaskDelay(pdMS_TO_TICKS(1000));
	}

}

void datalogger_task(void *pvParameters)
{
	datasensor_t data2;
	while(1)
	{
		if(xQueueReceive(buffer, ( void * ) &data2, portMAX_DELAY))
		{
			ESP_LOGI(TAG, "captured on: %d ms with state %d",data2.timestamp, data2.PIR_sensor);
		}
	}

}

void app_main(void)
{
	buffer = xQueueCreate(5, sizeof(datasensor_t));

	xTaskCreate(datastream_task, "data_stream", 2048, NULL, 5, NULL);
	xTaskCreate(datalogger_task, "data_logger", 2048, NULL, 5, NULL);

}
