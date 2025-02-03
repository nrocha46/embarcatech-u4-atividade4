#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/interrupt.h"
#include "ws2812b.pio.h"  // Biblioteca para controle dos LEDs WS2812

// Definindo os pinos
#define pino_botao_a 5        // Botão A no pino GPIO 5
#define pino_botao_b 6        // Botão B no pino GPIO 6
#define pino_led_r 11         // LED vermelho no pino GPIO 11
#define pino_led_g 12         // LED verde no pino GPIO 12
#define pino_led_b 13         // LED azul no pino GPIO 13
#define pino_led_ws 7         // WS2812 no pino GPIO 7

#define numero_leds 25        // Número de LEDs na matriz WS2812 (5x5)

#define tempo_debounce 200    // Tempo de debounce (200ms)

// Variáveis para armazenar os tempos das interrupções
absolute_time_t ultimo_tempo_interrupt_a = {0}; // Armazena o tempo da última interrupção do botão A
absolute_time_t ultimo_tempo_interrupt_b = {0}; // Armazena o tempo da última interrupção do botão B

volatile int numero_atual = 0; // Número exibido na matriz de LEDs

// Função de debouncing: evita múltiplas leituras rápidas do botão
bool debounce(absolute_time_t ultimo_tempo_interrupt, uint32_t tempo_debounce_ms) {
    absolute_time_t tempo_atual = get_absolute_time();
    if (absolute_time_diff_us(ultimo_tempo_interrupt, tempo_atual) > tempo_debounce_ms * 1000) {
        return true; // Retorna verdadeiro quando o tempo de debounce foi atingido
    }
    return false; // Retorna falso se o tempo de debounce não foi atingido
}

// Função para exibir um número na matriz de LEDs WS2812
void exibir_numero(int numero) {
    // Definir os padrões para os números de 0 a 9 na matriz 5x5
    uint8_t padroes_digitos[10][numero_leds] = {
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1}, // 0
        {0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0}, // 1
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1}, // 2
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1}, // 3
        {1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0}, // 4
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1}, // 5
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1}, // 6
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}, // 7
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1}, // 8
        {1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1}  // 9
    };

    // Loop para acender ou apagar os LEDs da matriz de acordo com o número
    for (int i = 0; i < numero_leds; i++) {
        if (padroes_digitos[numero][i] == 1) {
            ws2812_set_color(i, 0xFF0000); // Acende o LED na cor vermelha
        } else {
            ws2812_set_color(i, 0); // Apaga o LED
        }
    }

    ws2812_show(); // Atualiza a matriz de LEDs
}

// Função para o botão A (incrementa o número)
void interrupcao_botao_a(uint gpio, uint32_t eventos) {
    if (debounce(ultimo_tempo_interrupt_a, tempo_debounce)) {
        numero_atual = (numero_atual + 1) % 10; // Incrementa o número, vai de 0 a 9
        exibir_numero(numero_atual); // Atualiza a matriz de LEDs
    }
}

// Função para o botão B (decrementa o número)
void interrupcao_botao_b(uint gpio, uint32_t eventos) {
    if (debounce(ultimo_tempo_interrupt_b, tempo_debounce)) {
        numero_atual = (numero_atual - 1 + 10) % 10; // Decrementa o número, vai de 0 a 9
        exibir_numero(numero_atual); // Atualiza a matriz de LEDs
    }
}

// Função para piscar o LED vermelho
void piscar_led_vermelho() {
    while (1) {
        gpio_put(pino_led_r, 1); // Acende o LED vermelho
        sleep_ms(100);           // Espera 100ms
        gpio_put(pino_led_r, 0); // Apaga o LED vermelho
        sleep_ms(100);           // Espera 100ms
    }
}

int main() {
    // Inicializando GPIOs
    gpio_init(pino_led_r);           // Inicializa o LED vermelho
    gpio_set_dir(pino_led_r, GPIO_OUT);  // Configura o pino como saída

    gpio_init(pino_led_g);           // Inicializa o LED verde
    gpio_set_dir(pino_led_g, GPIO_OUT);  // Configura o pino como saída

    gpio_init(pino_led_b);           // Inicializa o LED azul
    gpio_set_dir(pino_led_b, GPIO_OUT);  // Configura o pino como saída

    // Inicializando a matriz de LEDs WS2812
    ws2812_init(pino_led_ws, numero_leds);

    // Inicializando os botões
    gpio_init(pino_botao_a);          // Inicializa o botão A
    gpio_set_dir(pino_botao_a, GPIO_IN);  // Configura o pino como entrada
    gpio_pull_up(pino_botao_a);       // Habilita o resistor pull-up
    gpio_set_irq_enabled_with_callback(pino_botao_a, GPIO_IRQ_EDGE_FALL, true, &interrupcao_botao_a); // Interrupção no botão A

    gpio_init(pino_botao_b);          // Inicializa o botão B
    gpio_set_dir(pino_botao_b, GPIO_IN);  // Configura o pino como entrada
    gpio_pull_up(pino_botao_b);       // Habilita o resistor pull-up
    gpio_set_irq_enabled_with_callback(pino_botao_b, GPIO_IRQ_EDGE_FALL, true, &interrupcao_botao_b); // Interrupção no botão B

    // Inicia o LED vermelho piscando
    piscar_led_vermelho();

    return 0; // Finaliza o programa
}
