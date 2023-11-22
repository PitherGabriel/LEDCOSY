#include "api_communication.h"
#include "wifi_connection.h"
#include "sensor.h"
#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/rtc_io.h"
#include <sys/time.h> 
#include "string.h"
#include "iot_servo.h"

static const char* TAG = "app_main";


#define SLEEP_TIME                  60   /*Sleep time in sec*/
#define GET_COMMAND_RESET            6   /*Number of wakes for controlling LED*/
/*Variable definition*/
static RTC_DATA_ATTR struct timeval sleep_enter_time;
RTC_DATA_ATTR int wakeup_counter = 0;
/*Function prototypes*/
void wakeup_reason(void);


void app_main(void)
{
    /*Sleep time*/
    //struct timeval now;
    //gettimeofday(&now, NULL);
    //int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    /*Get get up reason and update wakeup_counter*/
    wakeup_reason();
    wakeup_counter++;

    /*Connect Wifi*/
    connect_wifi();
    /*Init 12c*/
    ESP_ERROR_CHECK(i2cdev_init());

    /*Enable timer wakeup resource*/
    ESP_LOGI(TAG, "Enabling timer wakeup resource...");
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(SLEEP_TIME*1000000));
    /*If wakeup = n, request command for adjusting LED light*/
    if (wakeup_counter == GET_COMMAND_RESET)
    {
        /*Reset counter*/
        wakeup_counter=0;
        /*Get lighting command*/
        http_request_command();
    }
    else read_sensor(); //Keep sending sensor data
}     


void wakeup_reason(void){

    /*Chech wakeup cause and update wakeup_counter*/
    switch (esp_sleep_get_wakeup_cause())
    {
        case ESP_SLEEP_WAKEUP_TIMER:
            printf("Wake up number: %d\n", wakeup_counter);
            //printf("Wake up from timer. Time spent in deep sleep: %dms\n", sleep_time_ms);
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            printf("Not a deep sleep reset\n");
    }
}