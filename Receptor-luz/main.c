#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>

// Includes dos periféricos do Pico SDK
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"

// Nossos próprios arquivos de cabeçalho
#include "include/config.h"
#include "include/lora.h"
#include "include/display.h"
#include "include/led_rgb.h"

// --- Variáveis Globais ---
ssd1306_t display; // Objeto do Display

// << ESTRUTURA MODIFICADA >>
// Estrutura para armazenar um conjunto de dados recebidos
typedef struct {
    float environment;
    float temperatura;
    float umidade;
    float luminosidade;
    float gas; // << CAMPO ALTERADO: de 'pressao' para 'luminosidade'
} DadosRecebidos_t;

volatile bool novos_dados_recebidos = false;
volatile int ultimo_rssi = 0;
volatile uint32_t pacotes_recebidos = 0;
DadosRecebidos_t dados_atuais = {0.0f, 0.0f, 0.0f, 0.0f, 0.2f}; // << INICIALIZAÇÃO ATUALIZADA

// --- FUNÇÕES DE INICIALIZAÇÃO DE HARDWARE ---

/**
 * @brief Inicializa o barramento I2C1 para o display OLED.
 */
void setup_i2c_display() {
    i2c_init(I2C_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    printf("I2C1 (Display) inicializado nos pinos SDA=%d, SCL=%d.\n", I2C_SDA_PIN, I2C_SCL_PIN);
}

/**
 * @brief Inicializa o barramento SPI0 e os pinos GPIO associados para o LoRa.
 * ESTA FUNÇÃO É A CORREÇÃO CRÍTICA.
 */
void setup_spi_lora() {
    // Inicializa o periférico SPI em si
    spi_init(LORA_SPI_PORT, 5 * 1000 * 1000); // 5 MHz

    // Informa aos pinos GPIO para serem controlados pelo periférico SPI
    gpio_set_function(LORA_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LORA_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LORA_MISO_PIN, GPIO_FUNC_SPI);
    
    printf("SPI0 (LoRa) inicializado nos pinos SCK=%d, MOSI=%d, MISO=%d.\n", LORA_SCK_PIN, LORA_MOSI_PIN, LORA_MISO_PIN);
}


void on_lora_receive(lora_payload_t* payload) {
    // << LÓGICA DE DECODIFICAÇÃO ATUALIZADA >>
    // Tenta decodificar o novo formato "T:25.1,H:45.0,L:1234"
    int items_parsed = sscanf((const char*)payload->message, "E:%f,T:%f,H:%f,L:%f,G:%f",
                                &dados_atuais.environment, // Mantido para compatibilidade
                                &dados_atuais.temperatura,
                                &dados_atuais.umidade,
                                &dados_atuais.luminosidade,
                                &dados_atuais.gas); // << Variável alterada

    if (items_parsed == 5) { // Continua esperando 3 itens
        ultimo_rssi = payload->rssi;
        pacotes_recebidos++;
        novos_dados_recebidos = true; 
    } else {
        printf("WARN: Pacote LoRa recebido com formato inesperado: %.*s\n", payload->length, payload->message);
    }
}


// --- FUNÇÃO PRINCIPAL ---

int main() {
    stdio_init_all();
    sleep_ms(3000);

    // 1. Inicialização de Hardware
    printf("--- Iniciando Hardware do Receptor ---\n");
    rgb_led_init();
    setup_i2c_display();
    setup_spi_lora();
    printf("--------------------------------------\n\n");

    // 2. Inicialização de Drivers e Módulos
    display_init(&display);
    rgb_led_set_color(COR_LED_AMARELO);
    display_startup_screen(&display);

// Prepara a configuração para o módulo LoRa
lora_config_t config = {
    .spi_port = LORA_SPI_PORT,
    .interrupt_pin = LORA_INTERRUPT_PIN,
    .cs_pin = LORA_CS_PIN,
    .reset_pin = LORA_RESET_PIN,
    .freq = LORA_FREQUENCY,
    .tx_power = LORA_TX_POWER,
    .this_address = LORA_ADDRESS_RECEIVER
};

    if (!lora_init(&config)) {
        printf("ERRO FATAL: Falha na inicializacao do LoRa.\n");
        rgb_led_set_color(COR_LED_VERMELHO);
        while (1);
    }
     
    // 3. Finaliza a configuração e entra em modo de operação
    lora_on_receive(on_lora_receive);
    printf("Inicializacao completa. Endereco: #%d. Aguardando pacotes...\n", LORA_ADDRESS_RECEIVER);
    rgb_led_set_color(COR_LED_AZUL);
    display_wait_screen(&display);

    // 4. Loop Principal Infinito
    while (1) {
        if (novos_dados_recebidos) {
            
            // Variáveis locais para cópia segura dos dados
            DadosRecebidos_t dados_copiados;
            int rssi_copiado;
            uint32_t contador_copiado;
            
            // --- Seção Crítica ---
            gpio_set_irq_enabled(LORA_INTERRUPT_PIN, GPIO_IRQ_EDGE_RISE, false);
            novos_dados_recebidos = false;
            dados_copiados = dados_atuais;
            rssi_copiado = ultimo_rssi;
            contador_copiado = pacotes_recebidos;
            gpio_set_irq_enabled(LORA_INTERRUPT_PIN, GPIO_IRQ_EDGE_RISE, true);
            // --- Fim da Seção Crítica ---
            
            
            // 1. Feedback visual
            rgb_led_set_color(COR_LED_VERDE);
            
            // << CHAMADA DA FUNÇÃO DE DISPLAY ATUALIZADA >>
            // 2. Atualiza o display OLED com os novos dados
            display_update_data(&display,
                                dados_copiados.temperatura,
                                dados_copiados.umidade,
                                dados_copiados.luminosidade,
                                dados_copiados.gas, // << Variável alterada
                                dados_copiados.environment); // << Variável alterada
            
            // << LOG NO CONSOLE ATUALIZADO >>
            // 3. Imprime o log
            printf("Pacote #%lu | T:%.1f, H:%.0f, L:%.0f | G: %.2f | E: %.0f\n", 
                   contador_copiado, dados_copiados.temperatura,
                   dados_copiados.umidade, dados_copiados.luminosidade, dados_copiados.gas, // << Variável alterada
                   dados_copiados.environment);// << Variável alterada);
            
            // 4. Retorna o LED à cor de "pronto"
            sleep_ms(100);
            rgb_led_set_color(COR_LED_AZUL);
        }

        tight_loop_contents();
    }

    return 0;
}