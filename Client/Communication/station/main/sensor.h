#ifndef SENSOR_H
#define SENSOR_H
#include "scd30.h"

#define I2C_MASTER_SCL_IO           22      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           21      /*!< GPIO number used for I2C master data  */
    
void sensor_read(float *, float *, float *);

#endif //SENSOR_H