// Jakob Bleickert (Master Mechatronics)

#include <stdio.h>
#include "rmt_LED.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/rmt.h"
#include "led_strip.h"

#define RMT_TX_CHANNEL RMT_CHANNEL_0
#define CONFIG_EXAMPLE_STRIP_LED_NUMBER (15)
#define EXAMPLE_RMT_TX_GPIO (0)

const static char *TAG = "IOT";

led_strip_t * rmt_start (void)
{
    // Configuration
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(EXAMPLE_RMT_TX_GPIO, RMT_TX_CHANNEL);
    // set counter clock to 40MHz
    config.clk_div = 2;
    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
    // install ws2812 driver
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(CONFIG_EXAMPLE_STRIP_LED_NUMBER, (led_strip_dev_t)config.channel);
    led_strip_t *strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip)
    {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }

    // Visual Confirmation
    ESP_ERROR_CHECK(strip->set_pixel(strip, 14, 0, 50, 0));
    for (int i = 1; i <= 14; i++)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        rmt_write(i,strip);
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    for (int off = 0; off < 14; off++)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, off, 0, 0, 0));
    }
    ESP_ERROR_CHECK(strip->refresh(strip, 10));

    return strip;
}

void rmt_write (uint8_t travel, led_strip_t *strip)
{
    for (int i = 0; i < 14; i++)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, i, 100, 50, 0));
    }
    if (travel >= 12)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, 2, 50, 0, 0));
    }
    if (travel >= 13)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, 1, 50, 0, 0));
    }
    if (travel >= 14)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, 0, 50, 0, 0));
    }
    for (int off = ++travel; off <= 14; off++)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, abs(14-off), 0, 0, 0));
    }
    ESP_ERROR_CHECK(strip->refresh(strip, 10));
}

void rmt_turnoff (led_strip_t *strip)
{
    for (int off = 0; off < 14; off++)
    {
        ESP_ERROR_CHECK(strip->set_pixel(strip, off, 0, 0, 0));
    }
    ESP_ERROR_CHECK(strip->refresh(strip, 10));
}