/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"

const uint led_pin_red = 13;
const uint led_pin_blue = 12;

int main() {
    uint a = 100;
    
    gpio_init(led_pin_red);
    gpio_init(led_pin_blue);
    gpio_set_dir(led_pin_red, GPIO_OUT);
    gpio_set_dir(led_pin_blue, GPIO_OUT);
    stdio_init_all();
    while (true) {
        a++;
        if ( a % 2)
          printf("Blinking!\r\n");
        gpio_put(led_pin_red, true);
        gpio_put(led_pin_blue, true);
        sleep_ms(1000);
        gpio_put(led_pin_red, false);
        gpio_put(led_pin_blue, false);
        sleep_ms(1000);

    }
}
