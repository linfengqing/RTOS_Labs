/****************************************************************************
 * Copyright (C) 2020 by Fabrice Muller                                     *
 *                                                                          *
 * This file is useful for ESP32 Design course.                             *
 *                                                                          *
 ****************************************************************************/

/**
 * @file lab3-1_main.c
 * @author Fabrice Muller
 * @date 20 Oct. 2020
 * @brief File containing the lab3-1 of Part 3.
 *
 * @see https://github.com/fmuller-pns/esp32-vscode-project-template
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include "esp_log.h"

/* FreeRTOS.org includes. */
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "my_helper_fct.h"

static const char* TAG = "SEM";

/* Application constants */
#define STACK_SIZE     4096
#define TABLE_SIZE     400

/* Task Priority */
const uint32_t TIMER_TASK_PRIORITY = 5;
const uint32_t INC_TABLE_TASK_PRIORITY = 3;
const uint32_t DEC_TABLE_TASK_PRIORITY = 4;

/* Communications */
SemaphoreHandle_t xSemIncTab;
SemaphoreHandle_t xSemDecTab;

/* Tasks */
void vTaskTimer(void *pvParameters);
void vTaskIncTable(void *pvParameters);
void vTaskDecTable(void *pvParameters);

/* Datas */
int Table[TABLE_SIZE];
const uint32_t constNumber=7;

/* Main function */
void app_main(void) {

	/* Init Table */
	memset(Table, 0, TABLE_SIZE*sizeof(int));
	//for(int i=0;i<TABLE_SIZE;i++){
	//	Table[i]=i;
	//}
	/* Create semaphore */
	xSemIncTab=xSemaphoreCreateBinary();
	if(xSemIncTab==NULL){
		ESP_LOGE(TAG,"can't create semaphore xSemIncTab");
	}
	xSemDecTab=xSemaphoreCreateBinary();
	if(xSemDecTab==NULL){
		ESP_LOGE(TAG,"can't create semaphore xSemDecTab");
	}
	/* Stop scheduler */
	vTaskSuspendAll();

	vTaskSuspendAll();
	/* Create Tasks */
	xTaskCreatePinnedToCore(vTaskTimer,"Task Timer",STACK_SIZE,(void*)"Task Timer",TIMER_TASK_PRIORITY,NULL,CORE_0);
	xTaskCreatePinnedToCore(vTaskIncTable,"Task IncTable",STACK_SIZE,(void*)"Task IncTable",INC_TABLE_TASK_PRIORITY,NULL,CORE_0);
	xTaskCreatePinnedToCore(vTaskDecTable,"Task DecTable",STACK_SIZE,(void*)"Task DecTable",DEC_TABLE_TASK_PRIORITY,NULL,CORE_1);

	xTaskResumeAll();
	/* Continue scheduler */
	xTaskResumeAll();

	/* to ensure its exit is clean */
	vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

void vTaskTimer(void *pvParameters) {
	TickType_t xLastWakeTime;
	xLastWakeTime=xTaskGetTickCount();
	for(;;){
		DISPLAY("Task Timer : waiting computation");
		vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(250));
		COMPUTE_IN_TIME_MS(20);

		DISPLAY("Task Timer : give sem IncTab");
		xSemaphoreGive(xSemIncTab);
		DISPLAY("Task Timer : give sem DecTab");
		xSemaphoreGive(xSemDecTab);
	}
}

void vTaskIncTable(void *pvParameters) {
	int ActivationNumber=0;
	int index;
	for(;;){
		xSemaphoreTake(xSemIncTab,portMAX_DELAY);
		if(ActivationNumber==0){
			for(index=0;index<TABLE_SIZE;index++){
				Table[index]=Table[index]+constNumber;
			}
			DISPLAY("Task IncTable : start computation");
			COMPUTE_IN_TIME_MS(40);
			DISPLAY("Task IncTable : end computation");
			ActivationNumber=4;
		}
		else
			ActivationNumber=ActivationNumber-1;
	}
}

void vTaskDecTable(void *pvParameters) {
	int index;
	for(;;){
		xSemaphoreTake(xSemDecTab,portMAX_DELAY);
		for(index=0;index<TABLE_SIZE;index++){
			Table[index]=Table[index]-1;
		}
		DISPLAY("Task DecTable : start computation");
		COMPUTE_IN_TIME_MS(50);
		DISPLAY("Task DecTable : end computation");
	}
}



