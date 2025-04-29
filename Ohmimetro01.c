// Autor: Levi Silva Freitas
// Descrição: Ohmímetro com display OLED SSD1306 e ADC do Raspberry Pi Pico

// Bibliotecas necessárias para o funcionamento do código
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Definições de hardware
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define I2C_ADDRESS 0x3C

#define ADC_PIN 28

// Constantes
#define R_KNOWN 10000        // Resistor conhecido de 10kΩ
#define ADC_VREF 3.31         // Tensão de referência do ADC
#define ADC_RESOLUTION 4095   // Resolução do ADC (12 bits)

// Série E24 de resistores (5% de tolerância)
const int E24_SERIES[] = {
    10, 11, 12, 13, 15, 16, 18, 20, 22, 24, 27, 30,
    33, 36, 39, 43, 47, 51, 56, 62, 68, 75, 82, 91
};

// Nome das cores para o código de cores de resistores
const char* COLOR_NAMES[] = {
    "preto", "marrom", "vermelho", "laranja", "amarelo",
    "verde", "azul", "violeta", "cinza", "branco"
};

// Inicialização do display OLED
void init_display(ssd1306_t* display) {
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(display, WIDTH, HEIGHT, false, I2C_ADDRESS, I2C_PORT);
    ssd1306_config(display);
    ssd1306_fill(display, false);
    ssd1306_send_data(display);
}

// Inicialização do ADC
void init_adc() {
    adc_init();
    adc_gpio_init(ADC_PIN);
}

// Função para ler a média de 500 amostras do ADC
float read_adc_average() {
    adc_select_input(2); // Seleciona o canal correspondente ao GPIO28
    float sum = 0.0f;

    for (int i = 0; i < 500; i++) {
        sum += adc_read();
        sleep_ms(1);
    }

    return sum / 500.0f;
}

// Calcula o valor do resistor desconhecido baseado na média lida
float calculate_unknown_resistor(float adc_value) {
    return (ADC_RESOLUTION * R_KNOWN) / adc_value;
}

// Encontra o valor comercial mais próximo da série E24
int find_nearest_commercial_value(float value) {
    int decade = 1; // Fator de escala para ajustar o valor

    // Ajusta o valor para a faixa de 1 a 100
    while (value >= 100) {
        value /= 10;
        decade *= 10;
    }

    // Encontra o valor mais próximo na série E24
    int closest = E24_SERIES[0];
    float min_diff = fabs(value - E24_SERIES[0]);

    for (int i = 1; i < 24; i++) {
        float diff = fabs(value - E24_SERIES[i]);
        if (diff < min_diff) {
            closest = E24_SERIES[i];
            min_diff = diff;
        }
    }

    return closest * decade;
}

// Converte o valor da resistência em faixas de cores
void resistor_value_to_colors(int value, char* color1, char* color2, char* multiplier) {
    if (value < 1) value = 1;

    // Limita o valor a 99999
    int d1 = 0, d2 = 0, mult = 0;

    // Ajusta o valor para a faixa de 1 a 100
    while (value >= 100) {
        value /= 10;
        mult++;
    }

    d1 = value / 10;
    d2 = value % 10;

    if (d1 > 9) d1 = 9;
    if (d2 > 9) d2 = 9;
    if (mult > 9) mult = 9;

    // Converte os dígitos e o multiplicador para os nomes das cores
    strcpy(color1, COLOR_NAMES[d1]);
    strcpy(color2, COLOR_NAMES[d2]);
    strcpy(multiplier, COLOR_NAMES[mult]);
}

// Atualiza o conteúdo do display OLED
void update_display(ssd1306_t* display, const char* color1, const char* color2, const char* multiplier, const char* adc_str, const char* resistor_str, bool invert_colors) {
    ssd1306_fill(display, !invert_colors);
    ssd1306_rect(display, 3, 3, 122, 60, invert_colors, !invert_colors);
    ssd1306_line(display, 3, 37, 123, 37, invert_colors);

    // Cores
    ssd1306_draw_string(display, color1, 8, 6);
    ssd1306_draw_string(display, color2, 8, 15);
    ssd1306_draw_string(display, multiplier, 8, 24);

    // Valor de resistência
    ssd1306_draw_string(display, resistor_str, 85, 15);

    // Separadores
    ssd1306_line(display, 44, 37, 44, 60, invert_colors);
    ssd1306_line(display, 75, 3, 75, 36, invert_colors);

    // Valores do ADC e resistência
    ssd1306_draw_string(display, "ADC", 13, 41);
    ssd1306_draw_string(display, "Resisten.", 50, 41);
    ssd1306_draw_string(display, adc_str, 8, 52);
    ssd1306_draw_string(display, resistor_str, 59, 52);

    ssd1306_send_data(display);
}

// Função principal
int main() {
    stdio_init_all();

    // Inicializa o display e o ADC
    ssd1306_t display;
    init_display(&display);
    init_adc();

    // Ininicializa variáveis utilizadas no loop
    char adc_str[5];
    char resistor_str[8];
    char color1[10], color2[10], multiplier[10];
    bool invert_colors = true;

    while (true) {
        float adc_average = read_adc_average(); // Lê a média do ADC
        float r_unknown = calculate_unknown_resistor(adc_average); // Calcula o resistor desconhecido
        int r_commercial = find_nearest_commercial_value(r_unknown); // Encontra o valor comercial mais próximo

        resistor_value_to_colors(r_commercial, color1, color2, multiplier); // Converte o valor em cores

        sprintf(adc_str, "%1.0f", adc_average); // Formata o valor do ADC
        sprintf(resistor_str, "%d", r_commercial); // Formata o valor do resistor

        update_display(&display, color1, color2, multiplier, adc_str, resistor_str, invert_colors); // Atualiza o display

        invert_colors = !invert_colors;
        sleep_ms(700);
    }
}
