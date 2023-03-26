// Jakob Bleickert (Master Mechatronics)

#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "led_strip.h"

#ifdef __cplusplus
extern "C" {
#endif

led_strip_t * rmt_start (void);
void rmt_write (uint8_t travel, led_strip_t *strip);
void rmt_turnoff (led_strip_t *strip);

#ifdef __cplusplus
}
#endif