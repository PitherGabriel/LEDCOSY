#include "api_communication.h"
#include "wifi_connection.h"
#include "sensor.h"
#include <stdio.h>
#include "servo.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_sleep.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "driver/rtc_io.h"
#include <sys/time.h> 
#include "string.h"

static const char* TAG = "app_main";
#define GET_COMMAND_RESET            6   /*Number of wakes for controlling LED*/
/*Variable definition*/
RTC_DATA_ATTR int wakeup_counter = 0;
long DEEP_SLEEP_TIME_SEC = 1800;   /*Sleep time in sec*/
/*Function prototypes*/
void wakeup_reason(void);
void test_servo(void);
void test_command(void);


void app_main(void)
{
    //test_servo();
    /*Sleep time*/
    //struct timeval now;
    //gettimeofday(&now, NULL);
    //int sleep_time_ms = (now.tv_sec - sleep_enter_time.tv_sec) * 1000 + (now.tv_usec - sleep_enter_time.tv_usec) / 1000;

    /*Update wakeup_counter and get wake up reason*/
    wakeup_counter++;
    wakeup_reason();

    /*Connect Wifi*/
    connect_wifi();
    /*Init 12c*/
    ESP_ERROR_CHECK(i2cdev_init());

    /*Enable timer wakeup resource*/
    ESP_LOGI(TAG, "Enabling timer wakeup resource...");
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(1000000ULL * DEEP_SLEEP_TIME_SEC));
    //test_command();
    /*Checking whether to request command for adjusting LED light or keep sending sensor data*/
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

    /*Chech wakeup cause*/
    switch (esp_sleep_get_wakeup_cause())
    {
        case ESP_SLEEP_WAKEUP_TIMER:
            printf("Wake up from timer. Wake up number: %d\n", wakeup_counter);
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            printf("Not a deep sleep reset. Wake up number: %d\n", wakeup_counter);
    }
}

void test_servo(void){
    ESP_LOGI(TAG, "Test servo called.\n");
    move_servo();
}
void test_command(void){
    
    http_request_command();
}