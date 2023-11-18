/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "api_communication.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "sdkconfig.h"

static const char *TAG = "API Communication";

#define REQUEST_LENGTH 400
#define WEB_PATH_LENGTH 80

static const char* GET_PREFIX = "GET ";
static const char* POST_PREFIX = "POST ";

static const char* HTTP_REQUEST_SUFFIX = " HTTP/1.1\r\n"
    "Host: "WEB_SERVER":"WEB_PORT"\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n";

static const char* JSON_CONTENT_SUFFIX = "Content-Type: application/json\r\n"
                                         "Content-Length: ";

char request[REQUEST_LENGTH];
char web_path[WEB_PATH_LENGTH];



static void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[64];

    while(1) {
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);

        if(err != 0 || res == NULL) {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

        s = socket(res->ai_family, res->ai_socktype, 0);
        if(s < 0) {
            ESP_LOGE(TAG, "... Failed to allocate socket.");
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... allocated socket");

        if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
            ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }

        ESP_LOGI(TAG, "... connected");
        freeaddrinfo(res);

        if (write(s, request, strlen(request)) < 0) {
            ESP_LOGE(TAG, "... socket send failed");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... socket send success");

        struct timeval receiving_timeout;
        receiving_timeout.tv_sec = 5;
        receiving_timeout.tv_usec = 0;
        if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                sizeof(receiving_timeout)) < 0) {
            ESP_LOGE(TAG, "... failed to set socket receiving timeout");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        ESP_LOGI(TAG, "... set socket receiving timeout success");

        /* Read HTTP response */
        do {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf)-1);
            for(int i = 0; i < r; i++) {
                putchar(recv_buf[i]);
            }
        } while(r > 0);

        ESP_LOGI(TAG, "... done reading from socket. Last read return=%d errno=%d.", r, errno);
        close(s);
        vTaskDelete(NULL);
    }
}

void encode_sensor_data(char *buffer, float temp, float hum, float co2){
    sprintf(buffer, "{\"temperature\":%.2f,\"humidity\":%.2f,\"co2\":%.2f}", temp, hum, co2);
}

 void http_send_data(float temp, float hum, float co2)
{
    // Reset request and web path memory
    memset(request, 0, REQUEST_LENGTH);
    memset(web_path,0, WEB_PATH_LENGTH);
    // Buffer for JSON data
    char json_data[100];

    //Encode sensor data
    encode_sensor_data(json_data, temp, hum, co2);
    //
    int json_size_bytes = strlen(json_data);
    char jsonsize_str[5];
    itoa(json_size_bytes, jsonsize_str, 10);   

    strcpy(request, POST_PREFIX);
    strcpy(web_path, SENSOR_PREFIX_PATH);

    strcat(request, web_path);
    strcat(request, HTTP_REQUEST_SUFFIX);
    strcat(request, JSON_CONTENT_SUFFIX);
    strcat(request,jsonsize_str);
    strcat(request, "\r\n");
    strcat(request, "\r\n");
    strcat(request, json_data);

    printf(request);

    xTaskCreate(&http_get_task, "http_post_task", 4096, NULL, 5, NULL);
 }

 void http_request_command(void){

    // Reset request and web path memory
    memset(request, 0, REQUEST_LENGTH);
    memset(web_path,0, WEB_PATH_LENGTH);
    
    strcpy(request, GET_PREFIX);
    strcpy(web_path, FORECASTING_PREFIX_PATH);

    strcat(request,web_path);
    strcat(request,HTTP_REQUEST_SUFFIX);

    printf(request);

    xTaskCreate(&http_get_task, "http_get_task", 4096, NULL, 5, NULL);
 }