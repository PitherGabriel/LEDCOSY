#include "api_communication.h"
#include "wifi_connection.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/i2c.h"
#include "string.h"
#include "scd30.h"
#include "iot_servo.h"

static const char* TAG = "app_main";

#ifndef APP_CPU_NUM
#define APP_CPU_NUM PRO_CPU_NUM
#endif

#define I2C_MASTER_SCL_IO           22      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21      /*!< GPIO number used for I2C master data  */

// static const char* jsonSensorData = "{"
//     "\"temperature\":"+ String(temperature)+",
//     "\"humidity\": 60.0,"
//     "\"co2\": 400"
// "}";

void task(void *pvParameters)
{
    i2c_dev_t dev = {0};

    ESP_ERROR_CHECK(scd30_init_desc(&dev, 1, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO));

    uint16_t version, major_ver, minor_ver;
    ESP_ERROR_CHECK(scd30_read_firmware_version(&dev, &version));

    major_ver = (version >> 8) & 0xf;
    minor_ver = version & 0xf;

    ESP_LOGI(TAG, "SCD30 Firmware Version: %d.%d", major_ver, minor_ver);

    //ESP_LOGI(TAG, "Starting continuous measurement");
    //ESP_ERROR_CHECK(scd30_trigger_continuous_measurement(&dev, 0));
    ESP_LOGI(TAG, "Starting measurement");
    const TickType_t xDelay = 5000 / portTICK_PERIOD_MS;
    float co2, temperature, humidity;
    bool data_ready;
    while (1)
    {
        vTaskDelay(xDelay);
        scd30_get_data_ready_status(&dev, &data_ready);

        if (data_ready)
        {
            esp_err_t res = scd30_read_measurement(&dev, &co2, &temperature, &humidity);
            if (res != ESP_OK)
            {
                ESP_LOGE(TAG, "Error reading results %d (%s)", res, esp_err_to_name(res));
                continue;
            }

            if (co2 == 0)
            {
                ESP_LOGW(TAG, "Invalid sample detected, skipping");
                continue;
            }

            //ESP_LOGI(TAG, "CO2: %.0f ppm", co2);
            //ESP_LOGI(TAG, "Temperature: %.2f °C", temperature);
            //ESP_LOGI(TAG, "Humidity: %.2f %%", humidity);

            //Send data over
            ESP_LOGI(TAG, "Sending data to server...");
            http_send_data(temperature, humidity, co2);
        }
    }
}

void app_main(void)
{
    connect_wifi();
    ESP_ERROR_CHECK(i2cdev_init());
    xTaskCreatePinnedToCore(task, "test", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL, APP_CPU_NUM);
}              