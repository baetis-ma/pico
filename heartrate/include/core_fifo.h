static void core1_entry();

static void core1_str_setup(char *outstring) {
    void core1_sio_irq() {
        int cnt = 0;
        while (multicore_fifo_rvalid()){
            while(1) {
                outstring[cnt] = (char)multicore_fifo_pop_blocking();
                if (outstring[cnt] == 0) {cnt = 0; break; }
                ++cnt;
            }
        }
        multicore_fifo_clear_irq();
    }
    multicore_fifo_clear_irq();
    irq_set_exclusive_handler(SIO_IRQ_PROC1, core1_sio_irq);
    irq_set_enabled(SIO_IRQ_PROC1, true);
}

static void core0_str_setup(char *outstring) {
    void core0_sio_irq() {
        int cnt = 0;
        while (multicore_fifo_rvalid()){
            while(1) {
                outstring[cnt] = (char)multicore_fifo_pop_blocking();
                if (outstring[cnt] == 0) {cnt = 0; break; }
                ++cnt;
            }
        }
        multicore_fifo_clear_irq();
    }
    stdio_init_all();
    multicore_launch_core1(core1_entry); 
    irq_set_exclusive_handler(SIO_IRQ_PROC0, core0_sio_irq);
    irq_set_enabled(SIO_IRQ_PROC0, true);
}
