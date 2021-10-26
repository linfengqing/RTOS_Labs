#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "esp_log.h"

/* FreeRTOS.org includes. */
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "my_helper_fct.h"

/* Default stack size for tasks */
static const uint32_t STACK_SIZE = 4000;

/* Task Priority */
static const uint32_t T1_PRIO = 2;
static const uint32_t T2_PRIO = 2;
static const uint32_t T3_PRIO = 3;

/*The depth of message queue is 5*/
static const uint32_t MESS_MAX_MSG = 5;

/*The tag*/
static const char* TAG = "MsgQ";

/*The queue handle*/
QueueHandle_t xMsgQ;

/* The task1 function. */
void vTaskFunction1(void *pvParameters);
/* The task2 function. */
void vTaskFunction2(void *pvParameters);
/* The task3 function. */
void vTaskFunction3(void *pvParameters);

/**
 * @brief Starting point function
 * 
 */

void app_main(void) {

	printf("Hello ESP32 !\n");

	// Create a queue capable of containing 5 uint32_t values.
	xMsgQ=xQueueCreate(MESS_MAX_MSG,sizeof(uint32_t));

	if(xMsgQ==NULL){
		ESP_LOGE(TAG,"MsgQ cannot be created");
	}

	vTaskSuspendAll();

    BaseType_t result;

	result=xTaskCreatePinnedToCore(vTaskFunction1, "Task 1", STACK_SIZE, (void*)"Task 1", T1_PRIO, NULL,CORE_1);
	if(result!=pdPASS){
		ESP_LOGE(TAG,"Task 1 cannot be created");
	}

	result=xTaskCreatePinnedToCore(vTaskFunction2, "Task 2", STACK_SIZE, (void*)"Task 2", T2_PRIO, NULL,CORE_0);
	if(result!=pdPASS){
		ESP_LOGE(TAG,"Task 2 cannot be created");
	}

	result=xTaskCreatePinnedToCore(vTaskFunction3, "Task 3", STACK_SIZE, (void*)"Task 3", T3_PRIO, NULL,CORE_0);
	if(result!=pdPASS){
		ESP_LOGE(TAG,"Task 2 cannot be created");
	}

	xTaskResumeAll();
	vTaskDelete(NULL);
	
}

void vTaskFunction3(void *pvParameters){
	for(;;){
		// Task blocked furing 100 ms = 10 ticks
		DISPLAY("Task 3 is waiting");
		vTaskDelay(pdMS_TO_TICKS (100)) ;
		DISPLAY("Task 3 is running");
		//Compute time : 2 ticks or 20 ms
		COMPUTE_IN_TICK(2);
	}
	
}

void vTaskFunction1(void *pvParameters){
	uint32_t value=0;
	TickType_t xLastWakeTime;

	//Initialize the xLastWakeTime variable with the current time
	xLastWakeTime = xTaskGetTickCount();
	
	// Post in a message queue to the Task 2
	for(;;){
		value++;
		// 0 means no timeout
		uint32_t result = xQueueSend(xMsgQ,&value,0) ;
		// Check result
		if (result==pdPASS){
			DISPLAY("Posted message %d in queue to Task 2",value);
		}
		else
			DISPLAY("Failed message %d in queue to Task 2",value);
		// Compute time : 4 ticks or 40 ms
		COMPUTE_IN_TIME_MS(40) ;
		// block periodically : 500 ms
		vTaskDelayUntil (&xLastWakeTime,pdMS_TO_TICKS(500)) ;
	}
	
}

void vTaskFunction2(void *pvParameters){
	uint32_t value=0;
	for(;;){
		// Wait for message,timeout = portMAX_DELAY
		uint32_t result=xQueueReceive(xMsgQ,&value,portMAX_DELAY);
		// display task number and message
		if (result==pdPASS){
			DISPLAY ("Task 2 well receive value %d",value);
			// Compute time : 3 ticks or 30 ms
			COMPUTE_IN_TIME_MS(30) ;
		}
		
	}
		
}

