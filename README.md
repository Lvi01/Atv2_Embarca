# Ohmímetro com Display OLED SSD1306 e ADC do Raspberry Pi Pico

## Descrição
Projeto de um **ohmímetro digital** utilizando:
- **Raspberry Pi Pico** como microcontrolador
- **Display OLED SSD1306** para exibição dos valores
- Leitura da resistência desconhecida usando o **ADC interno**
- Identificação automática do valor comercial mais próximo (série **E24**)
- Exibição do **código de cores** correspondente ao resistor

## Funcionalidades
- Mede a resistência de um componente desconhecido baseado em um divisor de tensão.
- Calcula e exibe o valor mais próximo da série E24 (5% de tolerância).
- Mostra também o código de cores correspondente ao valor encontrado.
- Atualiza o display a cada ~700 ms com efeito de inversão de cores.

## Hardware Necessário
- Raspberry Pi Pico
- Display OLED SSD1306 (I2C)
- Resistor conhecido de **10 kΩ**
- Protoboard e jumpers
- Resistores variados para teste

## Conexões
| Pino Pico | Função       | Ligação OLED |
|:----------|:-------------|:------------|
| GPIO14    | I2C SDA      | SDA          |
| GPIO15    | I2C SCL      | SCL          |
| 3V3(OUT)  | Alimentação  | VCC          |
| GND       | Terra        | GND          |
| GPIO28    | Leitura ADC  | Divisor de tensão |

## Dependências
- `pico/stdlib.h`
- `hardware/adc.h`
- `hardware/i2c.h`
- Biblioteca personalizada para OLED: `lib/ssd1306.h`
- Fonte personalizada: `lib/font.h`

**Obs**: As bibliotecas `ssd1306` e `font` devem estar disponíveis no projeto.

## Como Funciona
1. Inicializa o display OLED e o ADC do Pico.
2. Lê 500 amostras do ADC para maior precisão.
3. Calcula o valor do resistor desconhecido usando a fórmula do divisor de tensão.
4. Ajusta o valor para o resistor mais próximo da série E24.
5. Converte esse valor para o código de cores.
6. Atualiza o display com os valores e cores.

## Fórmulas Utilizadas
- **Cálculo do Resistor Desconhecido**:
  \[
  R_{desconhecido} = \frac{R_{conhecido} \times V_{adc}}{V_{ref} - V_{adc}}
  \]
  (ajustado para os parâmetros do Pico)

## Autor
Levi Silva Freitas
