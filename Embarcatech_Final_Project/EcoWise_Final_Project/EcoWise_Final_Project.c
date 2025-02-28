#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h" // Biblioteca para o I2C
#include "hardware/adc.h" // Biblioteca para leitura do ADC
#include "hardware/pwm.h" // Biblioteca para o PWM do Buzzer
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
#define DHT_PIN 20 

// Pino do sensor capacitivo de umidade do solo
#define SOIL_SENSOR_PIN 28 

// Pinos do LED RGB
#define LED_VERMELHO_PIN 13
#define LED_VERDE_PIN 11
#define LED_AZUL_PIN 12

// Pinos dos botões
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6

// Pino do Buzzer
#define BUZZER_PIN 21
#define FREQUENCIA_BUZZER 2000  // 2 kHz (som médio audível)

// Limites de umidade do solo (percentuais aproximados)
#define SOIL_DRY_THRESHOLD 30
#define SOIL_WET_THRESHOLD 70

// Variável global para o display OLED
ssd1306_t display;

// Variável global para o sensor DHT11
dht_t sensor;

// Prototipação das funções
void inicializa_perifericos(void);
void atualiza_led_rgb(int temperatura);
void exibe_no_oled(float temperatura, float umidade, float umidade_solo);
void verifica_umidade_solo(float umidade_solo);
void iniciar_buzzer(uint pin, uint freq);
void parar_buzzer(uint pin);


int main() {
    float temperatura = 0.0f, umidade = 0.0f;
    float umidade_solo = 0.0f;

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
            // Leitura do sensor de umidade do solo
            adc_select_input(0); // ADC2
            uint16_t leitura_adc = adc_read();
            printf("Valor ADC bruto: %u\n", leitura_adc);
            umidade_solo = 100.0f * (1.0f - ((float)leitura_adc / 4095.0f)); // Converte para percentual

            if (umidade_solo < 30.0) {
             printf("Status: Solo seco\n");
            } else if (umidade_solo < 60.0) {
                printf("Status: Solo ideal\n");
            } else {
                printf("Status: Solo encharcado\n");
            }

            // Exibe os valores no console
            printf("Temperatura: %.1f °C | Umidade: %.1f %% | Umidade Solo: %.1f %%\n", temperatura, umidade, umidade_solo);

            // Atualiza os LEDs e o OLED com base nos valores
            atualiza_led_rgb((int)temperatura);
            exibe_no_oled(temperatura, umidade, umidade_solo);

            // Verifica a condição da umidade do solo
            verifica_umidade_solo(umidade_solo);
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

    // Inicializa o Buzzer
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);
    gpio_put(BUZZER_PIN, 0);

    // Inicializa os botões
    gpio_init(BUTTON_A_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);

    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_B_PIN);

    // Inicializa o barramento I2C
    i2c_init(I2C_PORT, 400 * 1000); // Velocidade do I2C: 400 kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    // Inicializa o ADC para o sensor capacitivo
    adc_init();
    adc_gpio_init(SOIL_SENSOR_PIN);

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

void exibe_no_oled(float temperatura, float umidade, float umidade_solo) {
    char buffer[32];
    char status_solo[32]; // Declaração da variável para armazenar o estado do solo

    // Determina o status do solo com base na umidade
    if (umidade_solo < SOIL_DRY_THRESHOLD) {
        snprintf(status_solo, sizeof(status_solo), "Solo: Seco");
    } else if (umidade_solo > SOIL_WET_THRESHOLD) {
        snprintf(status_solo, sizeof(status_solo), "Solo: Encharcado");
    } else {
        snprintf(status_solo, sizeof(status_solo), "Solo: Ideal");
    }

    ssd1306_clear(&display);

    // Exibe a temperatura
    snprintf(buffer, sizeof(buffer), "Temp: %.1f C", temperatura);
    ssd1306_draw_string(&display, 0, 0, 1, buffer);

    // Exibe a umidade do ar
    snprintf(buffer, sizeof(buffer), "Umid Ar: %.1f %%", umidade);
    ssd1306_draw_string(&display, 0, 16, 1, buffer);

    // Exibe a umidade do solo
    snprintf(buffer, sizeof(buffer), "Umid Solo: %.1f %%", umidade_solo);
    ssd1306_draw_string(&display, 0, 32, 1, buffer);

    // Exibe o status do solo
    ssd1306_draw_string(&display, 0, 48, 1, status_solo);

    ssd1306_show(&display);
}

// Verifica a umidade do solo e alerta se necessário
void verifica_umidade_solo(float umidade_solo) {
    char buffer[32];
    char status_solo[32];

    // Determina o status do solo
    if (umidade_solo < SOIL_DRY_THRESHOLD) {
        snprintf(status_solo, sizeof(status_solo), "Solo: Seco");

        // Exibe mensagem no OLED para solo seco
        ssd1306_clear(&display);
        ssd1306_draw_string(&display, 0, 0, 1, "Alert: Solo seco!");
        ssd1306_draw_string(&display, 0, 16, 1, "Regou o solo hoje?");
        ssd1306_draw_string(&display, 0, 32, 1, "Press A: Sim, B: Nao");
        ssd1306_show(&display);

        // Alerta visual e sonoro
        for (int i = 0; i < 2; i++) {
            gpio_put(LED_VERMELHO_PIN, 1);
            gpio_put(BUZZER_PIN, FREQUENCIA_BUZZER);
            sleep_ms(500);
            gpio_put(LED_VERMELHO_PIN, 0);
            gpio_put(BUZZER_PIN, 0);
            parar_buzzer(BUZZER_PIN);
            sleep_ms(500);
        }

        // Aguarda interação do usuário
        while (1) {
            if (!gpio_get(BUTTON_A_PIN)) {
                // Botão A pressionado: Solo já regado
                ssd1306_clear(&display);
                ssd1306_draw_string(&display, 0, 0, 1, "Obrigado!");
                ssd1306_draw_string(&display, 0, 16, 1, "A plantinha agradece!");
                ssd1306_show(&display);
                sleep_ms(2000);
                break;
            } else if (!gpio_get(BUTTON_B_PIN)) {
                // Botão B pressionado: Solo não regado
                ssd1306_clear(&display);
                ssd1306_draw_string(&display, 0, 0, 1, "Regue o solo!");
                ssd1306_draw_string(&display, 0, 16, 1, "Sedeeeee!");
                ssd1306_show(&display);
                sleep_ms(2000);
                break;
            }
            sleep_ms(100); // Pequeno atraso para evitar leitura contínua
        }
    } else if (umidade_solo > SOIL_WET_THRESHOLD) {
        snprintf(status_solo, sizeof(status_solo), "Solo: Encharcado");

        // Exibe mensagem no OLED para solo encharcado
        ssd1306_clear(&display);
        ssd1306_draw_string(&display, 0, 0, 1, "Alert: Solo encharcado?");
        ssd1306_draw_string(&display, 0, 16, 1, "Verifique a drenagem.");
        ssd1306_draw_string(&display, 0, 32, 1, "Press A: Sim, B: Nao");
        ssd1306_show(&display);

        // Alerta visual e sonoro
        for (int i = 0; i < 2; i++) {
            gpio_put(LED_AZUL_PIN, 1);
            gpio_put(BUZZER_PIN, FREQUENCIA_BUZZER);
            sleep_ms(500);
            gpio_put(LED_AZUL_PIN, 0);
            gpio_put(BUZZER_PIN, 0);
            parar_buzzer(BUZZER_PIN);
        }

        // Aguarda interação do usuário
        while (1) {
            if (!gpio_get(BUTTON_B_PIN)) {
                // Botão A pressionado: Solo já drenado
                ssd1306_clear(&display);
                ssd1306_draw_string(&display, 0, 0, 1, "Obrigado!");
                ssd1306_show(&display);
                sleep_ms(2000);
                break;
            } else if (!gpio_get(BUTTON_A_PIN)) {
                // Botão B pressionado: Solo ainda encharcado
                ssd1306_clear(&display);
                ssd1306_draw_string(&display, 0, 0, 1, "Verifique drenagem!");
                ssd1306_show(&display);
                sleep_ms(2000);
                break;
            }
            sleep_ms(100); // Pequeno atraso para evitar leitura contínua
        }
    } else {
        snprintf(status_solo, sizeof(status_solo), "Solo: Ideal");
    }


}

// ** Inicia o buzzer com PWM **
void iniciar_buzzer(uint pin, uint freq) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    float clock_freq = 125000000.0;
    float divisor = clock_freq / (freq * 255);

    pwm_set_clkdiv(slice_num, divisor);
    pwm_set_wrap(slice_num, 255);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(pin), 127);
    pwm_set_enabled(slice_num, true);
}

// ** Para o buzzer **
void parar_buzzer(uint pin) {
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_set_enabled(slice_num, false);
}