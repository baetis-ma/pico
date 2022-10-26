// Show how to reconfigure and restart a channel in a channel completion
// interrupt handler.
//
//An uint32_t array of 2*NUMSAMP size is filled with data, once the dma
//is started pointing to this array 
// Our DMA channel will transfer data to a PIO state machine, 
// configured to serialise the raw bits that we push, one by one. We're going
// to use this to do some crude LED PWM by repeatedly sending values with the
// right balance of 1s and 0s. (note there are better ways to do PWM with PIO
// -- see the PIO PWM example).
//
// Once the channel has sent a predetermined amount of data, it will halt, and
// raise an interrupt flag. The processor will enter the interrupt handler in
// response to this, where it will reconfigure and restart the channel. This
// repeats.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "i2s.pio.h"

#define NUMSAMP 512
#define SAMPLE_RATE 0.100
#define SAMPLE_FREQ 22050
#define PIO_SERIAL_CLKDIV  125000000/(4*32*SAMPLE_FREQ)

float output[NUMSAMP];
float input[NUMSAMP];
#include "./include/fft.h"
#include "./include/fft.c"
#include "./include/fft_calc.c"

//globals
int dma_chan;
uint32_t i2s_data[2*NUMSAMP];
bool  trig_wait = false;

void dma_int_handler() {
    dma_hw->ints0 = 1u << dma_chan; // Clear the interrupt
    dma_channel_set_write_addr(dma_chan, &i2s_data[0], true); //Reset write ptr and restart dma

    //for(int i=0; i<4; i++){printf("0x%08x ",i2s_data[i]);} printf("\n");
    //printf("receive data array ptr   = 0x%x\n", i2s_data);
    trig_wait = true;
}

int main() {
    stdio_init_all();
    float next_time = 0.00;
    //setup pio output serialiser
    int i2s_ck = 2;
    int i2s_da = 3;
    int i2s_ws = 4;
    uint offset = pio_add_program(pio0, &i2s_program);
    i2s_program_init(pio0, 0, offset, i2s_ck, i2s_ws, i2s_da, PIO_SERIAL_CLKDIV);

    dma_chan = dma_claim_unused_channel(true);
    dma_channel_config c = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&c, DMA_SIZE_32);
    channel_config_set_write_increment(&c, true);
    channel_config_set_read_increment(&c, false);
    channel_config_set_dreq(&c, DREQ_PIO0_RX0);

    dma_channel_configure( dma_chan, &c,
        i2s_data ,         //dst
        &pio0_hw->rxf[0],  //src
        2*NUMSAMP,         //transfer count
        false              //Don't start yet
    );

    dma_channel_set_irq0_enabled(dma_chan, true);  //enable dma irq
    irq_set_exclusive_handler(DMA_IRQ_0, dma_int_handler); //set pico irq handler
    irq_set_enabled(DMA_IRQ_0, true);

    dma_int_handler(); // Manually call the handler once, to trigger the first transfer

    next_time = SAMPLE_RATE + (float)0.000001 * get_absolute_time(); 
    absolute_time_t start;
    while (true) {


       //printf("current time %12.5f\n", (float)0.000001 * get_absolute_time()); 
       //wait for next sample fininshed
       if(next_time < (float)0.000001 * get_absolute_time()){
           trig_wait = false;
           while(!trig_wait){sleep_us(50);}

	   //printf("%6.2f  ", next_time);
           //for(int i=0; i<4; i++){printf("0x%08x ",i2s_data[i]);} 
           for(int i=0; i<NUMSAMP; i++){
               input[i] = -1.0*((float)(int)i2s_data[1+2*i])/(1<<26);
               //printf("%7.4f ", input[i]);
	   } //printf("\n");
	   
           //start = get_absolute_time();
           fft_calc(input, output, NUMSAMP );

	   //printf("                   %d fft took   %.4fmsec\n", NUMSAMP,
           //    0.001 * absolute_time_diff_us(start, get_absolute_time()));
	   //printf("   fft  ");
           for (int a = 1; a < NUMSAMP/2; a++){ printf ("%7.4f ", output[a]);}
	   printf("\n");
	   //printf("\n\n");
           next_time = SAMPLE_RATE + next_time; 
       }
       sleep_us(100);
       
       //sleep_ms(50);
       //printf("write ptr = 0x%08x  tranfer cnt = 0x%02x\n", 
       //    dma_hw->ch[dma_chan].al3_write_addr, 
       //    dma_hw->ch[dma_chan].al3_transfer_count);
    }
}
