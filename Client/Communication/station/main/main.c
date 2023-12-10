#include "api_communication.h"
#include "wifi_connection.h"
#include "sensor.h"
#include <stdio.h>
#include "servo.h"
#include "main.h"
#include "math.h"

static const char* TAG = "app_main";
#define GET_COMMAND_RESET            4 //12   /*Number of wakes for controlling LED*/
#define GET_START_SYSTEM             1   /*When system starts*/

/*Variable definition*/
long DEEP_SLEEP_TIME_SEC = 30;          /*Sleep time in sec*/
RTC_DATA_ATTR int wakeup_counter = 0;
float co2, temperature, humidity = 0;
RTC_DATA_ATTR int angle = 30;         /*System starts at fixed angle*/
int action = 0;
int gain = 0;

/*Function prototypes*/
void wakeup_reason();
void calculate_angle(int , int);
void test(void);

void app_main(void)
{
    //test();
    struct timeval time1, time2;
    gettimeofday(&time1, NULL); 
    wakeup_reason();
    /*Connect Wifi*/
    connect_wifi();
    /*Init 12c*/
    ESP_ERROR_CHECK(i2cdev_init());
    
    /*Send sensor data*/
    sensor_read(&co2, &temperature, &humidity); //Read sensor data
    http_send_data(temperature, humidity, co2); //Send data to server

    /*Checking if request command for adjusting LED light*/
    if (wakeup_counter == GET_COMMAND_RESET)
    {
        /*Reset counter*/
        wakeup_counter = 0;
        /*Request lighting command from server*/
        http_request_command(&action, &gain);
        printf("Action: %d, gain: %d", action, gain);
        /*Calculate angle*/
        calculate_angle(action, gain);
        /*Move servo*/
        move_servo(angle);
    }

    if(!wakeup_counter) //When system starts up
    {
        /*Move servo to initial fixed position*/
        move_servo(angle);
        /*Enable timer wakeup resource*/
        //ESP_LOGI(TAG, "Enabling timer wakeup resource...");
        ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(1000000ULL * (DEEP_SLEEP_TIME_SEC)));
        esp_deep_sleep_start();

    }
    else
    {
        gettimeofday(&time2, NULL);
        float time_ms = (time2.tv_sec - time1.tv_sec)*1000 + (time2.tv_usec - time1.tv_usec)/1000;
        //printf("Wake up time spent: %f ms.,and %0.3f sec.\n",  time_ms, (time_ms/1000));
        ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(1000000ULL * (DEEP_SLEEP_TIME_SEC-(time_ms/1000))));
        esp_deep_sleep_start();
    }
}     


void wakeup_reason(){

    /*Chech wakeup cause*/
    switch (esp_sleep_get_wakeup_cause())
    {
        case ESP_SLEEP_WAKEUP_TIMER:
            wakeup_counter++;
            printf("Wake up from timer. Wake up number: %d\n.", wakeup_counter);
            break;
        case ESP_SLEEP_WAKEUP_UNDEFINED:
        default:
            printf("Not a deep sleep reset. Wake up number: %d\n.", wakeup_counter);
    }
}

void calculate_angle(int action, int gain){
    int degrees = 0;
    degrees = round((angle*gain)/100);
    if (action)
    {
        angle +=degrees;
        if (angle>225)
        {
            angle=225;
        }

    }
    else if (!action)
    {
        degrees = 0;
        angle += degrees;
    }
    
    else 
    {
        angle-=degrees;
        if (angle<195)
        {
            angle=195;
        }
    }
}

void test (void){
    http_request_command(&action, &gain);
    //move_servo(0);
    // while (1)
    // {
    //     /* code */
    // }
    
    ESP_ERROR_CHECK(esp_sleep_enable_timer_wakeup(1000000ULL * (DEEP_SLEEP_TIME_SEC)));
    esp_deep_sleep_start();
    //http_request_command(&action, &gain);
}