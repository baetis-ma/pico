#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

char outstring[2000];
#define I2C_SDA    20
#define I2C_SCK    21
#define I2C_POWER  15
#include "./include/i2c.h"
#include "./include/ssd1306.h"
#define GATEDRIVE_PWM 16
#define PWM_LEN 1024

#define CURRENT       0.5
#define LOAD_RES      1.01
#define BATT_CUTOFF   2.5
#define MAX_LEN       128
#define UPDATE_RATE   10

float adc0, adc1, adc2;
void readadcs() {
    adc_select_input(0);
    sleep_us(100);
    adc0 = 5.24 * adc_read() * 3.3f / (1 << 12);
    sleep_us(100);

    adc_select_input(1);
    sleep_us(100);
    adc1 = adc_read() * 3.3f / (1 << 12);
    sleep_us(100);

    adc_select_input(2);
    sleep_us(100);
    adc2 = adc_read() * 3.3f / (1 << 12);
    sleep_us(100);
}

int main(void){
    stdio_init_all();

    //setup everthing
    i2c_start();
    ssd1306_init();
    //i2c_scan();

    char disp_string[256] = "|4 Battery||4  Dischage||4   Test||";
    ssd1306_text(disp_string);
    sleep_ms(2000);

    gpio_set_function(GATEDRIVE_PWM, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_wrap(slice_num, PWM_LEN);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
    pwm_set_enabled(slice_num, true);

    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_gpio_init(28);


    int number = 0; 
    float lbattv[MAX_LEN], lcurr[MAX_LEN]; 
    float rseries, vmax;
    for(int cnt = 0.5 * PWM_LEN; cnt < PWM_LEN; cnt = cnt + 10) {
         pwm_set_chan_level(slice_num, PWM_CHAN_A, cnt);
         sleep_us(10);
         readadcs();
         lbattv[number] = adc0;
         lcurr[number] = (adc1-adc2)/LOAD_RES;
         printf("%9.3f    %9.3f\n", adc0, (adc1 - adc2)/LOAD_RES);
         ++number;
         sleep_ms(50);
     }
     printf("e\n");
     pwm_set_chan_level(slice_num, PWM_CHAN_A, 0.4 * PWM_LEN);
     sleep_ms(10);
     //very half assed fit
     rseries = (lbattv[3] - lbattv[number - 3]) / (lcurr[number - 3] - lcurr[3]);
     vmax = lbattv[3];

     //discharge loop
     absolute_time_t start;
     uint64_t microsecs=0, lastmicrosecs=0;
     float ah = 0, wh = 0;
     float battv[MAX_LEN], curr[MAX_LEN]; 
     float abattv=0, acurr=0; 
     int gatevolt = 0, gatecontrol = 0.6 * PWM_LEN;
     int outrate = UPDATE_RATE;
     int time[MAX_LEN];
     int atime=0, div = 1, vlow = 0;
     int counter = 0, counterstop = 1000000;
     start = get_absolute_time();
     while (counter < counterstop) {
        readadcs();
        gatevolt = (int)(0.5*(PWM_LEN/3.3)*(CURRENT-((adc1 - adc2)/LOAD_RES))*LOAD_RES);
        gatecontrol = gatecontrol + gatevolt;

        if(gatecontrol < 0) gatecontrol = 0; 
        if(gatecontrol > PWM_LEN) gatecontrol = PWM_LEN;
        if(adc0 < BATT_CUTOFF) ++vlow;
        if(vlow > 5) gatecontrol = 0.2 * PWM_LEN;
        if(vlow == 5) { counterstop = counter + counter/10; } //add 10% after low volt detect

        pwm_set_chan_level(slice_num, PWM_CHAN_A, gatecontrol);

        lastmicrosecs = microsecs;
        microsecs = absolute_time_diff_us(start, get_absolute_time()),
        ah = ah + 0.000001 * (float)(microsecs - lastmicrosecs) * ((adc1 - adc2)/LOAD_RES)/3600; 
        wh = wh + adc0 * 0.000001 * (float)(microsecs - lastmicrosecs) * ((adc1 - adc2)/LOAD_RES)/3600; 
        //enter data into out array - keep shrinking timescale to fit data (avg all points)
        atime = atime + microsecs/1000000;
        abattv = abattv + adc0;
        acurr = acurr + (adc1 - adc2)/LOAD_RES;
        //compress time scale by factor of two when max length reached
        if (counter/div == MAX_LEN) {
             div = div * 2;
             for(int i = 0; i < MAX_LEN/2; i++) {
                 time[i]  = (time[2*i]  + time[2*i+1]) / 2;
                 battv[i] = (battv[2*i] + battv[2*i+1]) / 2;
                 curr[i]  = (curr[2*i]  + curr[2*i+1]) / 2;
             }
         }
         if (counter%div == (div-1)) {
              time[(counter/div)%MAX_LEN] = atime / div;
              battv [(counter/div)%MAX_LEN] = abattv / div;
              curr[(counter/div)%MAX_LEN] = acurr / div;
              atime = 0;
              abattv = 0;
              acurr = 0;
          }
	 
	  //print to serial and oled
          if(microsecs/1000000 > outrate) {
              outrate = outrate + UPDATE_RATE;
              for(int i = 0; i < (counter/div)%MAX_LEN; i++) {
                     printf("%7d %7.3f %7.3f   %9.4f %9.4f  \n", time[i], battv[i], curr[i], ah, wh);
              }
              printf("e\n");
              //write to oled display
              float curr = (adc1-adc2)/LOAD_RES;
              if (curr < 0) curr = 0;
              if(vlow < 5) {
                   sprintf(disp_string,  
                        "|4 %.3f Volt||4 %.3f Amps||4 %5d mAH||1 %.3fhours %.2fohm",
                        adc0, curr, (int)(1000*ah), ((float)(microsecs/1000000)/3600), rseries);
               }
               else {
                   sprintf(disp_string,  
                        "|4 Test Done||4 %4d mAH|||1 Vbatt=%.2fv %.2fh|1 %.3fhours %.2fohm",
                        (int)(1000*ah), adc0, rseries, ((float)(microsecs/1000000)/3600));
               }
               ssd1306_text(disp_string);
          }
          counter++;
          sleep_ms(1000);
     }
     pwm_set_chan_level(slice_num, PWM_CHAN_A, 0x00);
     printf("Program Exit - Vbattery < : %d\n");
}

