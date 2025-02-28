#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define ADC_PIN 28  // Definição do pino ADC

int main() {
    stdio_init_all();
    sleep_ms(2000);  // Aguarda a inicialização da comunicação serial

    printf("Iniciando leitura do ADC no GPIO28...\n");

    // Inicializa o ADC e seleciona o canal correspondente ao GPIO28
    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(2); // GPIO28 corresponde ao canal ADC2

    while (1) {
        uint16_t valor_adc = adc_read();  // Lê o valor bruto do ADC (0 - 4095)
        float tensao = valor_adc * 3.3 / 4095;  // Converte para tensão (escala de 0V a 3.3V)

        printf("Valor ADC bruto: %d | Tensão: %.2fV\n", valor_adc, tensao);

        sleep_ms(1000);  // Aguarda 1 segundo antes da próxima leitura
    }

    return 0;
}