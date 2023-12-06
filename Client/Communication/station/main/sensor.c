#include "sensor.h"
#include "esp_log.h"
#include "esp_err.h"

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
    bool data_ready;
    while (1)
    {
        scd30_get_data_ready_status(&dev, &data_ready);
        if (data_ready)
        {
            esp_err_t res = scd30_read_measurement(&dev, co2, temperature, humidity);
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

            printf("Sensor data read.\n");
            break;
        }
    }
}
