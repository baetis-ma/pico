//subroutine accepts disp_string format to write test on oled display
//char '|' makes new line, first char 1,2,4 sets size of text
void display_text(char *disp_string){
   int line = 0; int size = 1; char f0; char f1; char ft;
   int framebuf_ptr = 0;

   for (int n=0; n< strlen( disp_string); n++) {
      if(disp_string[n] == '|') { framebuf_ptr = 128 * ++line; }
      else if(disp_string[n]=='1' && framebuf_ptr%128 == 0){size = 1;++framebuf_ptr;}
      else if(disp_string[n]=='2' && framebuf_ptr%128 == 0){size = 2;++framebuf_ptr;}
      else if(disp_string[n]=='4' && framebuf_ptr%128 == 0){size = 4;++framebuf_ptr;}
      else if(size == 1){
         for(int a=0; a < 5; a++){
             framebuffer[framebuf_ptr++] = fonttable5x7[(a + 5*(disp_string[n]-' ' ))%1024]; } 
         framebuf_ptr++; }
      else if(size == 2 || size == 4){ //twice as wide
          for(int a=0; a < 5; a++){
             framebuffer[framebuf_ptr++] = fonttable5x7[(a + 5*(disp_string[n]-' ' ))%1024]; 
             framebuffer[framebuf_ptr++] = fonttable5x7[(a + 5*(disp_string[n]-' ' ))%1024]; 
             if(size == 4){ //twice as big
                 f0 = 0; f1 = 0; 
                 ft = fonttable5x7[a + 5*(disp_string[n]-' ')] ;
                 if((ft >> 7) & 1) f0 = 0xc0; 
                 if((ft >> 6) & 1) f0 = f0 + 0x30; 
                 if((ft >> 5) & 1) f0 = f0 + 0x0c; 
                 if((ft >> 4) & 1) f0 = f0 + 0x03; 
                 if((ft >> 3) & 1) f1 = 0xc0; 
                 if((ft >> 2) & 1) f1 = f1 + 0x30; 
                 if((ft >> 1) & 1) f1 = f1 + 0x0c; 
                 if((ft >> 0) & 1) f1 = f1 + 0x03; 
                 framebuffer[framebuf_ptr - 1] = f1; 
                 framebuffer[framebuf_ptr - 2] = f1; 
                 framebuffer[128+ (framebuf_ptr - 1)] = f0; 
                 framebuffer[128+ (framebuf_ptr - 2)] = f0; 
             }
          }
       framebuf_ptr++; }
   }
}

