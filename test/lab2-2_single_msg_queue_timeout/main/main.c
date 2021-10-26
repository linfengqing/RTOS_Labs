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



/*The depth of message queue*/
static const uint32_t MESS_MAX_MSG = 5;

/*The tag*/
static const char* TAG = "MsgQ";
static const char* TAG2= "MsgTimeOut";

/*Queue handle*/
QueueHandle_t xMsgQ;

/*The default stack size*/
static const uint32_t STACK_SIZE = 4000;

/*The Task Priority*/
static const uint32_t PRIO_T1=2;
static const uint32_t PRIO_T2=2;
static const uint32_t PRIO_T3=3;

/*The Task function*/
void vTaskFunction1(void* pvParameters);
void vTaskFunction2(void* pvParameters);
void vTaskFunction3(void* pvParameters);

void app_main(void){
    DISPLAY("start of app_main task ,the priority is %d",uxTaskPriorityGet(NULL));
    
    /*Create queueu*/
    xMsgQ=xQueueCreate(MESS_MAX_MSG,sizeof(uint32_t));
    if(xMsgQ==NULL){
        ESP_LOGE(TAG,"Message Queue can not be created !");
    }
    
    vTaskSuspendAll();

    /*Creation of tasks*/
    BaseType_t result;

    result=xTaskCreatePinnedToCore(vTaskFunction1,"Task 1",STACK_SIZE,(void *)"Task 1",PRIO_T1,NULL,CORE_1);
    if(result!=pdPASS){
        ESP_LOGE(TAG,"Task 1 can not be created !");
    }
    result=xTaskCreatePinnedToCore(vTaskFunction2,"Task 2",STACK_SIZE,(void *)"Task 2",PRIO_T2,NULL,CORE_0);
    if(result!=pdPASS){
        ESP_LOGE(TAG,"Task 2 can not be created !");
    }
    result=xTaskCreatePinnedToCore(vTaskFunction3,"Task 3",STACK_SIZE,(void *)"Task 3",PRIO_T3,NULL,CORE_0);
    if(result!=pdPASS){
        ESP_LOGE(TAG,"Task 3 can not be created !");
    }

    xTaskResumeAll();

    DISPLAY("==== Exit APP_MAIN ====");
    vTaskDelete(NULL);
}

void vTaskFunction3 (void* pvParameters){
    for(;;){
        DISPLAY("Task 3 : waiting computation");
        vTaskDelay(pdMS_TO_TICKS(100));
        DISPLAY("Task 3 : start computation");
        COMPUTE_IN_TICK(2);
    }

}


void vTaskFunction1(void* pvParameters){
    uint32_t mess=0;
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    for(;;){
        uint32_t result = xQueueSend(xMsgQ,&mess,0);
        if(result==pdTRUE){
            DISPLAY("Posted message %d in queue to Task 2",mess);
        }
        else
            DISPLAY("Failed message %d in queue to Task 2",mess);   
        COMPUTE_IN_TICK(4);
        vTaskDelayUntil(&xLastWakeTime,pdMS_TO_TICKS(500));      
    }
}

void vTaskFunction2(void* pvParameters){
    uint32_t mess=0;
    for(;;){
        uint32_t result = xQueueReceive(xMsgQ,&mess,pdMS_TO_TICKS(300));
        if(result==pdTRUE){
            DISPLAYI(TAG2, "Task 2 well receive message %d",mess);
            COMPUTE_IN_TICK(3);
        }
        else {
            DISPLAYE(TAG2, "Task 2, TimeOut");
            COMPUTE_IN_TICK(1);
        }
    }
}