/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

char outstring[2048];
int runload;
float current;
#include "./include/tcp_server.h"

int main() {

    stdio_init_all();
    //setup wifi
    if (cyw43_arch_init()) { printf("failed to initialise\n"); return 1; }
    cyw43_arch_enable_sta_mode();

    printf("Connecting to WiFi...\n");
    if (cyw43_arch_wifi_connect_timeout_ms("troutstream", "password", CYW43_AUTH_WPA2_AES_PSK, 30000)) {
        printf("failed to connect.\n");
        return 1;
    } else { printf("Connected.\n"); }
    printf("ip address %s\n", ip4addr_ntoa(netif_ip4_addr(netif_list)));

    //const uint LED_PIN = 25;
    //gpio_init(LED_PIN);
    //gpio_set_dir(LED_PIN, GPIO_OUT);

    int counter = 0;
    int led_state = 0;
    while(1) {
        mesgsent = 0;
        sprintf(outstring, "count = %d", counter);
        run_tcp_server_start();
	printf("tcp resp --> %s\n", tcpresponce);
	int ret = sscanf(tcpresponce, "getData?runload=%d", &led_state);
        if (ret == 1) cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_state);
        printf("\n");
	sleep_ms(25);
	++counter;
    }
    cyw43_arch_deinit();
    return 0;
}
