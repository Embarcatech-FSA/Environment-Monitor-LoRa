#ifndef CONFIG_H
#define CONFIG_H

// --- CONFIGURAÇÃO I2C ---
#define I2C_BAUDRATE        (400 * 1000)

// --- Barramento I2C 1: Para o Display OLED ---
#define I2C1_PORT           i2c1
#define I2C1_SDA_PIN        14
#define I2C1_SCL_PIN        15
#define DISPLAY_I2C_ADDR    0x3C
#define DISPLAY_WIDTH       128
#define DISPLAY_HEIGHT      64

// --- Barramento I2C 0: Apenas para os Sensores ---
#define I2C0_PORT           i2c0
#define I2C0_SDA_PIN        0
#define I2C0_SCL_PIN        1
#define AHT20_I2C_ADDR      0x38

// ==========================================================
// --- Configurações do LoRa (COM PINOS E PARÂMETROS FINAIS) ---
#define LORA_SPI_PORT       spi0
#define LORA_SCK_PIN        18
#define LORA_MOSI_PIN       19
#define LORA_MISO_PIN       16
#define LORA_CS_PIN         17
#define LORA_INTERRUPT_PIN  8  // DIO0
#define LORA_RESET_PIN      20

// --- Parâmetros da Comunicação LoRa ---
#define LORA_FREQUENCY      915.0 // Frequência ajustada para o padrão brasileiro
#define LORA_TX_POWER       20

// --- Endereços LoRa ---
#define LORA_ADDRESS_TRANSMITTER 1
#define LORA_ADDRESS_RECEIVER    2

#endif // CONFIG_H