#include "sensor.h"
#include "esp_log.h"
#include "esp_err.h"
#include "math.h"
static const char* TAG = "Sensor"; 


void sensor_read(float *co2, float *temperature, float *humidity)
{
    /*Sensor init*/
    i2c_dev_t dev = {0};
    ESP_ERROR_CHECK(scd30_init_desc(&dev, 1, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO));
    //uint16_t version, major_ver, minor_ver;
    //ESP_ERROR_CHECK(scd30_read_firmware_version(&dev, &version));

    //major_ver = (version >> 8) & 0xf;
    //minor_ver = version & 0xf;
    //ESP_LOGI(TAG, "SCD30 Firmware Version: %d.%d", major_ver, minor_ver);
    ESP_LOGI(TAG, "Starting measurement");
    //ESP_ERROR_CHECK(scd30_trigger_continuous_measurement(&dev, 0));
    bool data_ready;
    int error_flag;
    int retry_count = 0;
    while (1)
    {
        error_flag = 0;
        scd30_get_data_ready_status(&dev, &data_ready);
        ESP_LOGI(TAG, "Data Ready: %d", data_ready);
        if (retry_count >=5)
        {
            ESP_LOGE(TAG, "Max tries have reached. Skipping measurement");
            break;
        }
        
        if (data_ready)
        {
            esp_err_t res = scd30_read_measurement(&dev, co2, temperature, humidity);
            ESP_LOGI(TAG, "Reading Measurement: %d (%s)", res, esp_err_to_name(res));
            if (res != ESP_OK)
            {
                ESP_LOGE(TAG, "Error reading results %d (%s)", res, esp_err_to_name(res));
                error_flag = 1;
                continue;
            }
            if (isnan(*co2)||isnan(*temperature)||isnan(*humidity))
            {
                ESP_LOGW(TAG, "Null samples detected, skipping");
                error_flag = 1;
                continue;
            }
            
            if (*co2 == 0)
            {
                ESP_LOGW(TAG, "Invalid sample detected, skipping");
                error_flag = 1;
                continue;
            }
            if (!error_flag)
            {
            ESP_LOGI(TAG, "Sensor data read.\n");
            break;
            }
        }
        retry_count++;
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }
}
