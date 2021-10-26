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
const uint32_t INSPECT_TASK_PRIORITY = 4;
const uint32_t INC_TABLE_TASK_PRIORITY = 3;
const uint32_t DEC_TABLE_TASK_PRIORITY = 3;

/* Communications */
SemaphoreHandle_t xSemIncTab;
SemaphoreHandle_t xSemDecTab;
SemaphoreHandle_t xSemMutex;

/* Tasks */
void vTaskTimer(void *pvParameters);
void vTaskIncTable(void *pvParameters);
void vTaskDecTable(void *pvParameters);
void vTaskInspector(void *pvParameters);

/*constNumber in */
const uint32_t constNumber=7;

/* Datas */
int Table[TABLE_SIZE];

/* Main function */
void app_main(void) {

	/* Init Table */
	memset(Table, 0, TABLE_SIZE*sizeof(int));
	for(int i=0;i<TABLE_SIZE;i++) {
		Table[i]=i;
	}

	/* Create two separate semaphores */
	xSemIncTab=xSemaphoreCreateBinary();
	if(xSemIncTab == NULL){
		ESP_LOGE(TAG,"can't create semaphore");
		return;
	}

	xSemDecTab=xSemaphoreCreateBinary();
	if(xSemDecTab == NULL){
		ESP_LOGE(TAG,"can't create semaphore");
		return;
	}

	xSemMutex=xSemaphoreCreateMutex();
	if(xSemMutex == NULL){
		ESP_LOGE(TAG,"can't create semaphore");
		return;
	}

	/* Stop scheduler */
	vTaskSuspendAll();

	/* Create Tasks */
	xTaskCreatePinnedToCore(vTaskTimer, "Task Timer", STACK_SIZE, (void*)"Task Timer", TIMER_TASK_PRIORITY, NULL,CORE_0);
	xTaskCreatePinnedToCore(vTaskIncTable, "Task inc", STACK_SIZE, (void*)"Task inc", INC_TABLE_TASK_PRIORITY, NULL,CORE_0);
	xTaskCreatePinnedToCore(vTaskDecTable, "Task dec", STACK_SIZE, (void*)"Task dec", DEC_TABLE_TASK_PRIORITY, NULL,CORE_0);
	xTaskCreatePinnedToCore(vTaskInspector,"Task Inspector", STACK_SIZE, (void*)"Task Inspector", INSPECT_TASK_PRIORITY,NULL, CORE_1);

	/* Continue scheduler */
	xTaskResumeAll();

	/* to ensure its exit is clean */
	vTaskDelete(NULL);
}
/*-----------------------------------------------------------*/

void vTaskTimer(void *pvParameters) {
	TickType_t xLastWakeTime = xTaskGetTickCount();
	for(;;){
	    DISPLAY("Task Timer : waiting computation");
		vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(250));
		DISPLAY("Task Timer : start computation");
		COMPUTE_IN_TIME_MS(20);
		DISPLAY("Task Timer : give sem");
		xSemaphoreGive(xSemIncTab);
		xSemaphoreGive(xSemDecTab);
	}
	
}

void vTaskIncTable(void *pvParameters) {
	int index;
	int ActivationNumber;
	//uint32_t constNumber = *((uint32_t *)pvParameters);
	ActivationNumber=0;
	for(;;) {
		xSemaphoreTake(xSemIncTab,portMAX_DELAY);
		xSemaphoreTake(xSemMutex,portMAX_DELAY);
		if(ActivationNumber==0){
			DISPLAY("Task incTable : start computation");

			for(index=0;index<TABLE_SIZE;index++){
				Table[index]=Table[index]+constNumber;
			}
			DISPLAY("Task incTable : run computation");
			COMPUTE_IN_TIME_MS(50);
			ActivationNumber=4;
		}
		else{
			ActivationNumber=ActivationNumber-1;
			xSemaphoreGive(xSemMutex);}
		DISPLAY("Task incTable : end computation");
	}

	
}

void vTaskDecTable(void *pvParameters) {
	int index;
	for(;;){
		xSemaphoreTake(xSemDecTab,portMAX_DELAY);
		xSemaphoreTake(xSemMutex,portMAX_DELAY);
		DISPLAY("Task decTable : start computation");
		for(index=0;index<TABLE_SIZE;index++){
			Table[index]=Table[index]-1;
		}
		DISPLAY("Task decTable : run computation");
		COMPUTE_IN_TIME_MS(50);	
		xSemaphoreGive(xSemMutex);
		DISPLAY("Task decTable : end computation");
		
	}
}


void vTaskInspector(void *pvParameters) {
	int index;
	int reference;
	bool error;
	for(;;){
		xSemaphoreTake(xSemMutex,portMAX_DELAY);
		DISPLAY("Task Inspector is checking");
		reference=Table[0];
		error=false;
		for(index=1;index<TABLE_SIZE;index++) {
			COMPUTE_IN_TIME_US(100);
			if(Table[index]!=(reference+index)){
				error=true;
			}
		}
		DISPLAY("Task Inspectector ended its checking");
		if(error==true){
			ESP_LOGE(TAG,"Consistency error in the Table variable");
			exit(1);
		}

		xSemaphoreGive(xSemMutex);
		vTaskDelay(pdMS_TO_TICKS(20));
	}
}