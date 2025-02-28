#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#define LED_PIN 12
#define BTN_PIN 5

int main() {
    // Inicializa o pino do LED e configura como saída
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    // Inicializa o pino do botão e configura como entrada com resistor Pull-up
    gpio_init(BTN_PIN);
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_pull_up(BTN_PIN); // Configura Pull-up interno

    stdio_init_all();

    // Loop infinito para verificar o estado do botão e controlar o LED
    while (true) {
        // Verifica o estado do botão
        if (gpio_get(BTN_PIN) == 0) { // Pressionado (Pull-down, nível lógico baixo)
            printf("O botão foi pressionado (Pull-up detectado)\n");
            gpio_put(LED_PIN, true); // Liga o LED
            sleep_ms(200); // Tratamento básico para debounce
        } else { // Não pressionado (Pull-up, nível lógico alto)
            printf("O botão não foi pressionado\n");
            gpio_put(LED_PIN, false); // Desliga o LED
            sleep_ms(50); // Pequeno atraso para estabilidade
        }
    }
}
