#include "pico/stdlib.h"
#include <stdio.h>
#include <string.h>
#include "hardware/i2c.h"
#include "hardware/spi.h"
#include "./include/config.h"
#include "dados.h"
#include "include/lora.h"
#include "include/external/aht20.h"
#include "include/external/bh1750.h"
#include "include/display.h"
#include "include/joystick.h"
#include "include/mlp/mlp.h"


// -- Multilayer Perceptron
MLP mlp;
#define INPUT_LAYER_LEN 4
#define HIDDEN_LAYER_LEN 10
#define OUTPUT_LAYER_LEN 1

// Declaração dos objetos globais
ssd1306_t display;
DadosSistema_t dados_sistema;

// Função para inicializar o barramento I2C0 para os sensores
void setup_i2c0_sensores() {
    i2c_init(I2C0_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C0_SDA_PIN);
    gpio_pull_up(I2C0_SCL_PIN);
    printf("I2C0 (Sensores) inicializado.\n");
}

// Função para inicializar o barramento I2C1 para o display
void setup_i2c1_display() {
    i2c_init(I2C1_PORT, I2C_BAUDRATE);
    gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C1_SDA_PIN);
    gpio_pull_up(I2C1_SCL_PIN);
    printf("I2C1 (Display) inicializado.\n");
}


void setup_spi_lora() {
    // 1. Inicializa o periférico SPI na frequência desejada
    spi_init(LORA_SPI_PORT, 5 * 1000 * 1000); // 5 MHz

    // 2. Mapeia a função SPI para os pinos GPIO corretos
    gpio_set_function(LORA_SCK_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LORA_MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_function(LORA_MISO_PIN, GPIO_FUNC_SPI);
    
    // 3. Os pinos CS, RST, e IRQ são GPIOs normais, inicializados separadamente
    // (A biblioteca lora.c já faz isso, então não precisamos repetir aqui)
    
    printf("SPI0 (LoRa) e pinos GPIO associados inicializados.\n");
}


float getEnvironmentalConditions();
void trained_mlp_model();

int main() {
    stdio_init_all();
    sleep_ms(3000);

    // 1. INICIALIZA OS PERIFÉRICOS DE HARDWARE
    printf("--- Iniciando Hardware ---\n");
    setup_i2c0_sensores();
    setup_i2c1_display();
    setup_spi_lora();
    joystick_init(); // <<< INICIALIZA o Joystick/ADC
    printf("--------------------------\n\n");
    
    // 2. INICIALIZA OS DRIVERS
    display_init(&display);
    display_startup_screen(&display);

    // Zera a estrutura de dados
    dados_sistema = (DadosSistema_t){0};

    // Inicializa Sensores
    if (!aht20_init() || !bh1750_init(i2c0)) {
        printf("ERRO FATAL: Falha ao iniciar sensores I2C.\n");
        display_error_screen(&display, "Sensor Falhou");
        while(1);
    }
    printf("Sensores AHT20 e BH1750 OK.\n");

    // 3. INICIALIZA O LORA
    lora_config_t config = {
        .spi_port = LORA_SPI_PORT,
        .interrupt_pin = LORA_INTERRUPT_PIN,
        .cs_pin = LORA_CS_PIN,
        .reset_pin = LORA_RESET_PIN,
        .freq = LORA_FREQUENCY,
        .tx_power = LORA_TX_POWER,
        .this_address = LORA_ADDRESS_TRANSMITTER
    };

    if (!lora_init(&config)) {
        printf("ERRO FATAL: Falha na inicializacao do LoRa.\n");
        dados_sistema.lora_ok = false;
        display_error_screen(&display, "LoRa Falhou");
    } else {
        printf("LoRa inicializado com sucesso!\n");
        dados_sistema.lora_ok = true;
    }

    trained_mlp_model(); // Aplica o modelo treinado

    // --- LOOP PRINCIPAL ---
    while (1) {
        if (dados_sistema.lora_ok) {
            // Ler dados dos sensores I2C
            aht20_read_data(&dados_sistema.temperatura, &dados_sistema.umidade);
            dados_sistema.luminosidade = (float)bh1750_read_lux(i2c0);

            // <<< LÓGICA DO JOYSTICK IMPLEMENTADA AQUI >>>
            // 1. Lê o valor bruto do eixo X do joystick (0-4095)
            //    GPIO 26 corresponde ao canal ADC 0
            uint16_t joy_x_raw = read_adc(0); 
            // 2. Normaliza o valor para a faixa de 0 a 2000
            dados_sistema.gas = (joy_x_raw / 4095.0f) * 300.0f;

            dados_sistema.environment = getEnvironmentalConditions()*100.0;
            
            // Log no console
            printf("\n--- Novo Ciclo ---\n");
            printf("Dados: T=%.1fC, H=%.0f%%, L=%.0f Lux, Gas=%.0f ppm (raw=%d)\n", 
                dados_sistema.temperatura, dados_sistema.umidade, 
                dados_sistema.luminosidade, dados_sistema.gas, joy_x_raw);

            // Atualizar o display (que agora mostrará "Gas:")
            display_update_screen(&display, &dados_sistema);
            
            // Monta o payload para enviar com o novo dado
            char lora_buffer[64];
            snprintf(lora_buffer, sizeof(lora_buffer), "E:%.0f,T:%.1f,H:%.0f,L:%.0f,G:%.0f",
                     dados_sistema.environment,
                     dados_sistema.temperatura, 
                     dados_sistema.umidade, 
                     dados_sistema.luminosidade,
                     dados_sistema.gas); // << Adicionado o novo dado
            
            printf("Enviando pacote #%lu para #%d: \"%s\"\n", dados_sistema.pacotes_enviados + 1, LORA_ADDRESS_RECEIVER, lora_buffer);

            // Envia dados
            lora_send((uint8_t*)lora_buffer, strlen(lora_buffer), LORA_ADDRESS_RECEIVER);
            
            dados_sistema.pacotes_enviados++; // Mantemos o contador para o log
            printf("Envio concluido.\n");

        } else {
             printf("Sistema em estado de erro.\n");
        }

        sleep_ms(1000); // Reduzido o tempo de espera para ver o joystick mais reativo
    }

    return 0;
}

// Condições do ambiente
float getEnvironmentalConditions()
{
    float xMin[4] = {5.793664, 20.596113, 9.414636, 160.582703, };        // Valores mínimos de entrada no dataset
    float xMax[4] = {41.263657, 81.931076, 217.787125, 1086.463989};    // Valores máximos de entrada no dataset

    float yMin = 27.833267;
    float yMax = 75.133141;
    float X[4] = {dados_sistema.temperatura, dados_sistema.umidade, dados_sistema.gas, dados_sistema.luminosidade};        // Entrada atual

    // Normalização dos dados
    for (int j = 0; j < 4; j++) {
		X[j] = (X[j] - xMin[j]) / (xMax[j] - xMin[j]);
	}

    // Obtém a saída
    forward(&mlp, X);
    return mlp.output_layer_outputs[0];
}

void trained_mlp_model() {
    mlp.input_layer_length = INPUT_LAYER_LEN;
    mlp.hidden_layer_length = HIDDEN_LAYER_LEN;
    mlp.output_layer_length = OUTPUT_LAYER_LEN;

    float hidden_layer_weights[HIDDEN_LAYER_LEN][INPUT_LAYER_LEN+1] = {
        {-0.602525, -0.057686, -0.267446, -3.711378, 0.448500, },
        {0.179523, 1.331844, -0.373048, 0.562266, -0.152008, },
        {0.042265, -3.133777, 0.206005, 0.452716, 1.117836, },
        {6.563734, -0.047519, -0.127718, 0.279491, -4.153003, },
        {-0.059122, 0.405526, -0.397460, -0.176535, 0.472396, },
        {1.645131, 0.127165, 0.404172, -2.753453, 1.723942, },
        {-0.021187, 0.583820, 0.534648, 0.310444, 0.906271, },
        {0.422764, 0.320140, 0.575352, 0.593663, 0.178238, },
        {-0.304632, 0.353708, -0.144588, -0.147737, 0.855667, },
        {4.430942, -0.881249, -0.258215, 0.186538, -1.514328, },
    };

    float output_layer_weights[OUTPUT_LAYER_LEN ][HIDDEN_LAYER_LEN+1] = {
        {-2.507177, 0.432395, -1.575119, -3.147566, 0.194103, 2.344924, -0.599903, -0.117070, 0.138718, 2.783273, -0.512408, }
    };

    // Alocar pesos da hidden layer
    mlp.hidden_layer_weights = (float**) malloc(mlp.hidden_layer_length * sizeof(float*));
    for (int i = 0; i < mlp.hidden_layer_length; i++) {
        mlp.hidden_layer_weights[i] = (float*) malloc((mlp.input_layer_length + 1) * sizeof(float));
        for (int j = 0; j < (mlp.input_layer_length + 1); j++) {
            mlp.hidden_layer_weights[i][j] = hidden_layer_weights[i][j];
        }
    }

    // Alocar pesos da output layer
    mlp.output_layer_weights = (float**) malloc(mlp.output_layer_length * sizeof(float*));
    for (int i = 0; i < mlp.output_layer_length; i++) {
        mlp.output_layer_weights[i] = (float*) malloc((mlp.hidden_layer_length + 1) * sizeof(float));
        for (int j = 0; j < (mlp.hidden_layer_length + 1); j++) {
            mlp.output_layer_weights[i][j] = output_layer_weights[i][j];
        }
    }

    // Alocar saídas
    mlp.hidden_layer_outputs = (float*) malloc(mlp.hidden_layer_length * sizeof(float));
    mlp.output_layer_outputs = (float*) malloc(mlp.output_layer_length * sizeof(float));
}