// Jakob Bleickert (Master Mechatronics)

#include <stdio.h>
#include "i2c_sensor.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define I2C_MASTER_NUM I2C_NUM_0      /*!< I2C port number for master dev */
#define I2C_MASTER_TX_BUF_DISABLE 0   /*!< I2C master do not need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0   /*!< I2C master do not need buffer */
#define I2C_MASTER_FREQ_HZ 100000        /*!< I2C master clock frequency */
#define I2C_MASTER_SCL_IO 6           /*!<gpio number for i2c master clock  */
#define I2C_MASTER_SDA_IO 5           /*!<gpio number for i2c master data */
#define ESP_SLAVE_ADDR 0x10           /*!< ESP32 slave address, you can set any 7bit value */
#define WRITE_BIT  I2C_MASTER_WRITE   /*!< I2C master write */
#define READ_BIT   I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN   0x1            /*!< I2C master will check ack from slave*/
#define ACK_VAL        0x0 
#define NACK_VAL       0x1

const static char *TAG = "IOT";

uint8_t i2c_master_driver_initialize(void)
{
	i2c_config_t conf = 
	{
		.mode = 			I2C_MODE_MASTER,
		.sda_io_num = 		I2C_MASTER_SDA_IO,         	// select GPIO specific to your project
		.sda_pullup_en = 	GPIO_PULLUP_ENABLE,
		.scl_io_num = 		I2C_MASTER_SCL_IO,         	// select GPIO specific to your project
		.scl_pullup_en = 	GPIO_PULLUP_ENABLE,
		.master.clk_speed = I2C_MASTER_FREQ_HZ,  		// select frequency specific to your project
        // .clk_flags = 0,
	};

    uint8_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
	if (err != ESP_OK) 
    {
        return err;
    }
	return i2c_driver_install(I2C_MASTER_NUM, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

void i2c_slave_detection(void)
{
    uint8_t address;
    printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\r\n");
    for (int i = 0; i < 128; i += 16) {
        printf("%02x: ", i);
        for (int j = 0; j < 16; j++) {
            fflush(stdout);
            address = i + j;
            i2c_cmd_handle_t cmd = i2c_cmd_link_create();
            i2c_master_start(cmd);
            i2c_master_write_byte(cmd, (address << 1) | WRITE_BIT, ACK_CHECK_EN);
            i2c_master_stop(cmd);
            esp_err_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 50 / portTICK_RATE_MS);
            i2c_cmd_link_delete(cmd);
            if (ret == ESP_OK) {
                printf("%02x ", address);
            } else if (ret == ESP_ERR_TIMEOUT) {
                printf("UU ");
            } else {
                printf("-- ");
            }
        }
        printf("\r\n");
    }
    i2c_driver_delete(I2C_MASTER_NUM); 
}

uint8_t i2c_get_distance (void)
{
    uint8_t reg_data[7];
    esp_err_t ret = i2c_read(&reg_data[0]);
    uint8_t distance = 0;

    if (ret == ESP_OK) 
    {
        if (reg_data[2] < 10)
        {
            distance = (reg_data[2]);
            printf("Distance: %x \n", distance);
        }
        else if (reg_data[2] == 0x0a)
        {
            distance = 10;
            printf("Distance: 10 \n");
        }
        else if (reg_data[2] == 0x0b)
        {
            distance = 11;
            printf("Distance: 11 \n");
        }
        else if (reg_data[2] == 0x0c)
        {
            distance = 12;
            printf("Distance: 12 \n");
        }
        else if (reg_data[2] == 0x0d)
        {
            distance = 13;
            printf("Distance: 13 \n");
        }
        /* else if (reg_data[2] == 0x0e)
        {
            distance = 14;
            printf("Distance: 14 \n");
        }*/
        else
        {
            distance = 14;
            printf("Distance: 14 \n");
        }
    }
    else if (ret == ESP_ERR_TIMEOUT)
    {
        distance = 14;
        ESP_LOGW(TAG, "Bus is busy");
    }
    else 
    {
        distance = 14;
        ESP_LOGW(TAG, "Read failed");
    }

    return distance;
}


int8_t i2c_read(uint8_t *reg_data)
{
    uint16_t length = 7;
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ESP_SLAVE_ADDR << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x01, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, 0x02, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, length, ACK_CHECK_EN);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, ESP_SLAVE_ADDR << 1 | READ_BIT, ACK_CHECK_EN);

    if (length > 1) 
    {
        i2c_master_read(cmd, &(reg_data[0]), length - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, &(reg_data[0]) + length - 1, NACK_VAL);

    i2c_master_stop(cmd);
    int8_t ret = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}