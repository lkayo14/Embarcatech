#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "SSD1306/ssd1306.h" // Biblioteca para o display OLED
#include <dht.h>             // Biblioteca para o DHT11

// Definições do barramento I2C e OLED
#define SDA_PIN 14
#define SCL_PIN 15
#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_ADDRESS 0x3C // Endereço I2C do OLED
#define I2C_PORT i2c1     // Porta I2C usada

// Pino do sensor DHT11
#define DHT_PIN 20 // Defina o pino conectado ao DHT11

// Pinos do LED RGB
#define LED_VERMELHO_PIN 13
#define LED_VERDE_PIN 11
#define LED_AZUL_PIN 12

// Variável global para o display OLED
ssd1306_t display;

// Variável global para o sensor DHT11
dht_t sensor;

// Prototipação das funções
void inicializa_perifericos(void);
void atualiza_led_rgb(int temperatura);
void exibe_no_oled(float temperatura, float umidade);

int main() {
    float temperatura = 0.0f, umidade = 0.0f;

    // Inicializa comunicação serial
    stdio_init_all();

    // Aguarda a conexão USB
    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    printf("Inicializando o sistema...\n");

    // Inicializa periféricos
    inicializa_perifericos();

    // Inicializa o display OLED
    display.external_vcc = false;
    if (!ssd1306_init(&display, OLED_WIDTH, OLED_HEIGHT, OLED_ADDRESS, I2C_PORT)) {
        printf("Falha ao inicializar o OLED! Verifique as conexões.\n");
        while (1) sleep_ms(1000);
    }

    printf("Sistema inicializado com sucesso.\n");

    // Loop principal
    while (1) {
        // Inicia a medição do DHT11
        dht_start_measurement(&sensor);

        // Aguarda o término da medição e coleta os dados
        dht_result_t resultado = dht_finish_measurement_blocking(&sensor, &umidade, &temperatura);

        if (resultado == DHT_RESULT_OK) {
            // Exibe os valores no console
            printf("Temperatura: %.1f °C | Umidade: %.1f %%\n", temperatura, umidade);

            // Atualiza os LEDs e o OLED com base nos valores
            atualiza_led_rgb((int)temperatura);
            exibe_no_oled(temperatura, umidade);
        } else if (resultado == DHT_RESULT_TIMEOUT) {
            printf("Erro: Timeout ao ler o sensor.\n");
        } else if (resultado == DHT_RESULT_BAD_CHECKSUM) {
            printf("Erro: Checksum inválido.\n");
        }

        sleep_ms(2000); // Aguarda 2 segundos antes da próxima leitura
    }

    return 0;
}

// Inicializa os periféricos (GPIOs, I2C e pinos do LED)
void inicializa_perifericos(void) {
    // Inicializa GPIOs do LED RGB
    gpio_init(LED_VERMELHO_PIN);
    gpio_set_dir(LED_VERMELHO_PIN, GPIO_OUT);
    gpio_put(LED_VERMELHO_PIN, 0);

    gpio_init(LED_VERDE_PIN);
    gpio_set_dir(LED_VERDE_PIN, GPIO_OUT);
    gpio_put(LED_VERDE_PIN, 0);

    gpio_init(LED_AZUL_PIN);
    gpio_set_dir(LED_AZUL_PIN, GPIO_OUT);
    gpio_put(LED_AZUL_PIN, 0);

    // Inicializa o barramento I2C
    i2c_init(I2C_PORT, 400 * 1000); // Velocidade do I2C: 400 kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Inicializa o DHT11
    dht_init(&sensor, DHT11, pio0, DHT_PIN, true);
}

// Atualiza o LED RGB com base na temperatura
void atualiza_led_rgb(int temperatura) {
    if (temperatura < 20 && temperatura >= -6) {
        // Temperatura entre -6 °C e 20 °C: Azul
        gpio_put(LED_AZUL_PIN, 1);
        gpio_put(LED_VERDE_PIN, 0);
        gpio_put(LED_VERMELHO_PIN, 0);
    } else if (temperatura > 30) {
        // Temperatura acima de 30 °C: Vermelho
        gpio_put(LED_AZUL_PIN, 0);
        gpio_put(LED_VERDE_PIN, 0);
        gpio_put(LED_VERMELHO_PIN, 1);
    } else {
        // Temperatura entre 20 °C e 30 °C: Verde
        gpio_put(LED_AZUL_PIN, 0);
        gpio_put(LED_VERDE_PIN, 1);
        gpio_put(LED_VERMELHO_PIN, 0);
    }
}

// Exibe a temperatura e umidade no OLED
void exibe_no_oled(float temperatura, float umidade) {
    char buffer[32];

    ssd1306_clear(&display);

    // Exibe a temperatura
    snprintf(buffer, sizeof(buffer), "Temp: %.1f C", temperatura);
    ssd1306_draw_string(&display, 0, 0, 1, buffer);

    // Exibe a umidade
    snprintf(buffer, sizeof(buffer), "Umid: %.1f %%", umidade);
    ssd1306_draw_string(&display, 0, 16, 1, buffer);

    ssd1306_show(&display);
}
