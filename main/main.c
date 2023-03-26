// Messung des Federwegs mit dem ESP32-C3, ein RISC-V μC von Espressif
// Jakob Bleickert (Master Mechatronics)
// Abgabedatum: 12.02.2022

#include <stdio.h>
#include "esp_log.h"
#include "i2c_sensor.h"
#include "rmt_LED.h"
#include "led_strip.h"
#include "driver/rmt.h"
#include "driver/gpio.h"

esp_err_t gpio_pulldown_en(GPIO_NUM_2);
uint8_t calculate_offset (uint8_t travel_raw, double offset);

void app_main(void)
{  
    // double offset = 4;

    int16_t meas_points = 1000; 
    int16_t meas_data[meas_points];
    for (int i = 0; i < meas_points; i++)
    {
        meas_data[i] = -1;
    }

    while (1)
    {
        gpio_num_t pin = (gpio_num_t)(GPIO_NUM_2 & 0x1F);
        int state = (GPIO_REG_READ(GPIO_IN_REG)  >> pin) & 1U;
        if (state)
        {
            i2c_master_driver_initialize();
            led_strip_t *strip = rmt_start();
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            
            for (int i = 0; i < meas_points; i++)
            {
                meas_data[i] = -1;
            }
            int16_t counter = 0;

            while(state)
            {
                uint8_t distance = i2c_get_distance();
                uint8_t travel = abs(distance-14);
                // uint8_t travel_with_offset = calculate_offset (travel, offset);
                rmt_write(travel,strip);
                meas_data[counter] = travel;
                state = (GPIO_REG_READ(GPIO_IN_REG)  >> pin) & 1U;
                vTaskDelay(10 / portTICK_PERIOD_MS);
                counter++;
            }
            rmt_turnoff (strip);
        }
        vTaskDelay(2000 / portTICK_PERIOD_MS);

        printf("\n\nLast Measurement:\n");
        for (int i = 0; i < meas_points; i++)
        {
            if (meas_data[i] != -1)
            {
                printf("%d. Travel: %d | ", i, meas_data[i]);
            }
        }
    }
}

uint8_t calculate_offset (uint8_t travel_raw, double offset)
{
    uint8_t travel1 = 0;
    int8_t travel2 = travel_raw - offset;
    if (travel2 < 0)
    {
        travel1 = 0;
    }
    else
    {
        travel1 = travel2;
    }
    uint8_t travel = travel1*(14/(14-offset));

    return travel;
}