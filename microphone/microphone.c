#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"

#define I2C_SDA    20
#define I2C_SCK    21
#define I2C_POWER  15
//#include "./include/i2c.h"

#define FB_LEN  1024
uint8_t framebuffer[FB_LEN+1];
#define GATEDRIVE_PWM 16
#define PWM_LEN 1024
#include "./include/fft.h"
#include "./include/fft.c"
#include "./include/disp_wave.c"
#define NUMSAMP 256    
#define FFT_CEIL  25


void fft_calc(float *input, float *output, int num);
    
void fft_calc(float *input, float *output, int num)
{
    // Create fft plan and let it allocate arrays
    fft_config_t *fft_analysis = fft_init(num, FFT_COMPLEX, FFT_FORWARD, NULL, NULL);

    for (int k = 0 ; k < fft_analysis->size ; k++) {
      fft_analysis->input[2*k] = input[k];
      fft_analysis->input[2*k+1] = 0;
    }

    fft_execute(fft_analysis);
    for (int k = 0 ; k < fft_analysis->size/2 ; k++){
       output[k] = sqrt(fft_analysis->output[2*k]*fft_analysis->output[2*k]+
                        fft_analysis->output[2*k+1]*fft_analysis->output[2*k+1]);
    }
    //printf("\nraw fft output\n");
    //for (int k = 0 ; k < fft_analysis->size/2 ; k++) 
    //   printf("%4d %10.2f %10.2fj\n", k, fft_analysis->output[2*k], fft_analysis->output[2*k+1]); 
    fft_destroy(fft_analysis);
}

float adc0, adc1, adc2;
void readadcs() {
    adc_select_input(0);
    sleep_us(100);
    adc0 = adc_read() * 3.3f / (1 << 12);
    sleep_us(100);
    //adc0 = 5.24 * adc_read() * 3.3f / (1 << 12);

    //adc_select_input(1);
    //sleep_us(100);
    //adc1 = adc_read() * 3.3f / (1 << 12);

    //adc_select_input(2);
    //sleep_us(100);
    //adc2 = adc_read() * 3.3f / (1 << 12);
}


int main(void){
    stdio_init_all();
    absolute_time_t start;
    
    float output[NUMSAMP];
    float input[NUMSAMP];
    float regdata[NUMSAMP];
    char *disp_str_wave="4  Waveform|||||||1  ~12ms full scale";
    char *disp_str_spec="4  Spectrum||||||||";

    //gpio_init(10);
    //gpio_set_dir(10, GPIO_INT);
    //int mode = gpio_get(10;

    //i2c_start();
    //ssd1306_init();
    //i2c_scan();

    //char disp_string[256] = "|4 Battery||4  Dischage||4   Test||";
    //ssd1306_text(disp_string);
    //sleep_ms(2000);

    gpio_set_function(GATEDRIVE_PWM, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(0);
    pwm_set_wrap(slice_num, PWM_LEN);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, 1);
    pwm_set_enabled(slice_num, true);

    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
    adc_gpio_init(28);

    start = get_absolute_time();

    int cnt = 0;
    while (1) {
       //read mode pin on gpio 5
       //mode=gpio_get_level(MODE_GPIO);
       
       //read 128 bytes from ad convertor
       int mode = 0;
       for (int a = 0; a < NUMSAMP; a++) { 
           readadcs(); 
           regdata[a] = adc0;
//	    printf("%10d  %10.6f      %.3f %.3f %.3f\n", cnt, 
//               0.000001 * absolute_time_diff_us(start, get_absolute_time()),
//		adc0, adc1, adc2);
            //pwm_set_chan_level(slice_num, PWM_CHAN_A, (10 * cnt)%PWM_LEN);
	    //++cnt;
	}

       //clear frame buffer, setup display text string and write text fields to frame buffer
       //for(int i=0; i<FB_LEN; i++){ framebuffer[i]= (uint8_t)0x00; }
       //if (mode == 1) ssd1306_text(disp_str_wave); else ssd1306_text(disp_str_spec);

       //write waveform to oled display framebuffer
       //if (mode == 1) disp_wave((int)regdata);
       //else {
           for (int a = 0; a < NUMSAMP; a++) input[a] =  regdata[a];
           //for (int a = 0; a < NUMSAMP; a++){ 
           //    printf ("%.3f ", input[a]);
           //}
	   //printf("\n");
        start = get_absolute_time();
           fft_calc(input, output, NUMSAMP );
        printf("%d fft took   %10.6fsec\n", NUMSAMP, 
               0.000001 * absolute_time_diff_us(start, get_absolute_time()));
           for (int a = 1; a < NUMSAMP/2; a++){ 
               printf ("%.3f ", output[a]);
           }
	   printf(" 80\n");
	   sleep_ms(1);

           //for (int k=0; k<NUMSAMP/2; k++)
           //   printf("%3d %10.2f\n", k, 0.01*output[k]);

          //write frambuffer with box for each frequency of spectrum
        //  uint32_t scratch;
        //  for(int a=2; a<64;a++){
        //     scratch =0;
        //     for(int s=0; s<64; s++) if(0.3*output[a/2]/8>s)scratch=scratch+(1<<(31-s));
        //     framebuffer[6*128 + a +30] = scratch/(1<<24);
        //     framebuffer[5*128 + a +30] = (scratch%(1<<24))/(1<<16);
        //     framebuffer[4*128 + a +30] = (scratch%(1<<16))/(1<<8);
        //     framebuffer[3*128 + a +30] = scratch%(1<<8);
        //  }
       //write frame buffer to oled
       //uint8_t *data = malloc (1024+1);
       //data[0] = 0x40;
       //for(int i=0; i<1024+1; i++){ data[i+1] = framebuffer[i]; }
       //i2c_write_blocking( i2c_default, 0x3c, data, 1024/2+1, false);

       sleep_ms(1);
    }
}
