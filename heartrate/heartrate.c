#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "dsp/chebbpf.h"

//#include "pico/multicore.h"
//#include "hardware/irq.h"
//#include "./include/core_fifo.oh"

//rpi pico
//#define I2C_SDA    20
//#define I2C_SCK    21
//#define I2C_POWER  15
//seedi np2040
#define I2C_SDA    1
#define I2C_SCK    2
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

#define ARRAY_LEN 16

int main() {
    absolute_time_t start;
    float time, time_last = 0;
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
    data[1] = 0x58;   //red - ir
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
    float time_meas[ARRAY_LEN], timelast; 
    float red_val[ARRAY_LEN], ir_val[ARRAY_LEN], red_filt[ARRAY_LEN], ir_filt[ARRAY_LEN];
    int array_ptr = 0;
    start = get_absolute_time();
    int counter = 0;
    while (1) {

        //read fifo - first find len
        data[0] = FIFO_RD_PTR;
        i2c_write_blocking( i2c_default, 0x57, data, 1, true);
        i2c_read_blocking( i2c_default, 0x57, data, 3, false);
	size_t len = (data[0]%16) - data[2]%16;
        if ((int)len <= 0) len = len + 16;

	//printf("%10.6f  read prt = %2d  write ptr = %2d  len = %2d int = %2d\n", 
        //     time, data[0]%16, data[2]%16, (int)len, data[1]%16);
	//printf("%10.6f  len=%2d drop=%2d\n", time, len, data[1]%16);

        time_last = time;
	time = 0.000001 * absolute_time_diff_us(start, get_absolute_time());

        data[0] = FIFO_DATA;
        i2c_read_blocking( i2c_default, 0x57, data, 4*len, false);
	for(int i=0; i<len;i++) {
            printf("%10.3f %10.8f\n", 
               time_last + (i+1) * (time - time_last)/len,
               //((float)256*data[4*i+0]+data[4*i+1])/(1<<16),
               red_bpf(((float)256*data[4*i+2]+data[4*i+3])/(1<<16)),
               ((float)256*data[4*i+2]+data[4*i+3])/(1<<16));
	    //filter arrays here
            //time_meas[array_ptr%ARRAY_LEN] = time_last + (i+1) * (time - time_last)/len;
            //ir_val[array_ptr%ARRAY_LEN] = ((float)256*data[4*i+0]+data[4*i+1])/(1<<16);
            //red_val[array_ptr%ARRAY_LEN] = ((float)256*data[4*i+2]+data[4*i+3])/(1<<16);
	    //filter arrays here
	    //printf("ptr = %7d %7d  %7.3f\n", array_ptr,array_ptr%ARRAY_LEN, time_meas[array_ptr%ARRAY_LEN]);
	    //++array_ptr;
	//}
	//for(int i= 0; i<ARRAY_LEN;i++) printf("%7.3f ", time_meas[i]);
	//printf("\n");
	//for(int i= 0; i<ARRAY_LEN;i++) printf(" %.4f ", red_val[i]);
	//printf("\n");
	//for(int i= 0; i<ARRAY_LEN;i++) printf(" %.4f ", ir_val[i]);
	//printf("\n");
	//printf("\n");
	//for(int i= 0; i<ARRAY_LEN;i++){
        //    printf("%10.3f  %.7f\n", time_meas[i], red_val[i]);
	//    ++counter;
	}
	sleep_ms(50);
    }
}

