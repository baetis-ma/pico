void disp_wave(uint8_t regdata[128]){
       for(int a=0; a<128;a++){
           //throw away bottom two bits-only 32 pixels available in main display field 
           regdata[a] = regdata[a] / 4;
           char byte = 0x00;
           //looking at bits 2-4 set pixel bit
           if(regdata[a]%0x8 == 0x0)byte = 0x80;
           if(regdata[a]%0x8 == 0x1)byte = 0x40;
           if(regdata[a]%0x8 == 0x2)byte = 0x20;
           if(regdata[a]%0x8 == 0x3)byte = 0x10;
           if(regdata[a]%0x8 == 0x4)byte = 0x08;
           if(regdata[a]%0x8 == 0x5)byte = 0x04;
           if(regdata[a]%0x8 == 0x6)byte = 0x02;
           if(regdata[a]%0x8 == 0x7)byte = 0x01;
           //use bits 5-8 to move pixel byte (adc data with msb set outside field)
           framebuffer[(6-(regdata[a]/0x8)) * 128 + a] = byte;

       }
}
