#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"

//#include "pico/multicore.h"
//#include "hardware/irq.h"
//#include "./include/core_fifo.oh"

#define I2C_SDA    20
#define I2C_SCK    21
#define I2C_POWER  15
#include "./include/i2c.h"
//#include "./include/ssd1306.h"

//MAX30100 Registers
#define FIFO_RD_PTR    0x02
#define FIFO_WR_PTR    0x04
#define FIFO_DATA      0x05
#define MODE_CONFIG    0x06
#define SPO2_CONFIG    0x07
#define LED_CONFIG     0x09
#define TEMP_INT       0x16
#define TEMP_FRAC      0x27
#define REV_ID0        0xFE
#define REV_ID1        0xFF

int main() {
    absolute_time_t start;
    float time;
    float temperature;

    stdio_init_all();
    i2c_start();
    sleep_ms(3000);
    i2c_scan();
    //MAX30102 Configuration
    uint8_t data[128];
    data[0] = MODE_CONFIG;
    data[1] = (0<<7) + (0<<6) + (0<<3) + 3; //mode reg - shutdown, reset, temp, mode (3=spo2)
    data[2] = (1<<6) + (1<<2) + 1; //spo2 reg - dac res, rate(1=100), pulse(1=118) 
    i2c_write_blocking(i2c_default, 0x57, data, 3, false);
    data[0] = LED_CONFIG;
    data[1] = 0x88;
    i2c_write_blocking(i2c_default, 0x57, data, 2, false);

    //read product id
    data[0] = REV_ID0;
    i2c_write_blocking( i2c_default, 0x57, data, 1, true);
    i2c_read_blocking( i2c_default, 0x57, data, 2, false);
    printf("Produnct ID = %d %d\n", data[1], data[0]);

    //read temperature
    data[0] = MODE_CONFIG;
    data[1] = (0<<7) + (0<<6) + (1<<3) + 3; //mode reg - shutdown, reset, temp, mode (3=spo2)
    i2c_write_blocking(i2c_default, 0x57, data, 2, false);
    sleep_us(2);
    data[0] = TEMP_INT;
    i2c_write_blocking( i2c_default, 0x57, data, 1, true);
    i2c_read_blocking( i2c_default, 0x57, data, 2, false);
    temperature = 32 + 1.8 * data[0] + 1.8 * 0.0625 * data[1];
	printf("%10.6f  Temperature = %7.2f\n", time, temperature);

    int cnt = 0;
    start = get_absolute_time();
    while (1) {

        //read fifo
        data[0] = FIFO_RD_PTR;
        i2c_write_blocking( i2c_default, 0x57, data, 1, true);
        i2c_read_blocking( i2c_default, 0x57, data, 3, false);

	time = 0.000001 * absolute_time_diff_us(start, get_absolute_time());
	int len = data[2]%16 - data[0]%16;
        if (len < 0) len = len + 16;
	printf("%10.6f  read prt = %2d  write ptr = %2d  len = %2d int = %2d\n", 
             time, data[0]%16, data[2]%16, len, data[1]%16);
        data[0] = FIFO_DATA;
        i2c_read_blocking( i2c_default, 0x57, data, 10, false);



	sleep_ms(20);
    }
}

