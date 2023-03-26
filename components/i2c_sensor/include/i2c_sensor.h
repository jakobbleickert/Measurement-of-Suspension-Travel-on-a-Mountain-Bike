// Jakob Bleickert (Master Mechatronics)

#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t i2c_master_driver_initialize(void);
void i2c_slave_detection(void);
uint8_t i2c_get_distance (void);
int8_t i2c_read(uint8_t *reg_data);

#ifdef __cplusplus
}
#endif