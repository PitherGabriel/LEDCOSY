#include <stdio.h>
#include "servo.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


void servo_task(void *pvParameter){
    printf("Move servo task called\n");

    while (1)
    {
        vTaskDelay(5000/portTICK_PERIOD_MS);
        printf("Entering deep sleep...\n");
        //gettimeofday(&sleep_enter_time,NULL);
        esp_deep_sleep_start();
    }
    

}

void move_servo(void){
    xTaskCreate(&servo_task, "servo_task",4096, NULL, 5, NULL);
}