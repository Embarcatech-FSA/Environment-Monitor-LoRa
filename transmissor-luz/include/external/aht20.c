#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdio.h>

#include "aht20.h"
#include "include/config.h" // <<< Inclui as macros I2C0_PORT, AHT20_I2C_ADDR

bool aht20_init() {
    sleep_ms(40); // Tempo para o sensor ligar
    uint8_t status;
    
    // Usa o barramento I2C0 para os sensores
    int result = i2c_read_blocking(I2C0_PORT, AHT20_I2C_ADDR, &status, 1, false);
    
    if (result < 1) {
        printf("ERRO AHT20: Falha ao ler o status inicial.\n");
        return false;
    }

    // Se o bit de calibração (bit 3) não estiver setado, envie o comando de inicialização
    if (!(status & 0x08)) {
        uint8_t cmd_init[] = {0xBE, 0x08, 0x00};
        i2c_write_blocking(I2C0_PORT, AHT20_I2C_ADDR, cmd_init, 3, false);
        sleep_ms(10); 
        printf("AHT20 necessitou de inicializacao.\n");
    }

    printf("AHT20 pronto.\n");
    return true;
}

bool aht20_read_data(float *temperature, float *humidity) {
    uint8_t cmd_trigger[] = {0xAC, 0x33, 0x00};
    
    // Dispara a medição no barramento I2C0
    i2c_write_blocking(I2C0_PORT, AHT20_I2C_ADDR, cmd_trigger, 3, false);
    
    sleep_ms(80); // Esperar pela medição (datasheet recomenda ~75ms)

    uint8_t buffer[7];
    int result = i2c_read_blocking(I2C0_PORT, AHT20_I2C_ADDR, buffer, 7, false);

    // Se a leitura falhou ou o bit de "ocupado" (bit 7) está setado, retorne erro
    if (result < 7 || (buffer[0] & 0x80)) {
        printf("ERRO AHT20: Falha ao ler dados ou sensor ocupado.\n");
        return false;
    }

    // Fórmulas de conversão do datasheet do AHT20
    uint32_t raw_humidity = ((uint32_t)(buffer[1]) << 12) | ((uint32_t)(buffer[2]) << 4) | ((buffer[3] & 0xF0) >> 4);
    *humidity = ((float)raw_humidity / 1048576.0f) * 100.0f;

    uint32_t raw_temp = (((uint32_t)(buffer[3] & 0x0F)) << 16) | ((uint32_t)buffer[4] << 8) | buffer[5];
    *temperature = (((float)raw_temp / 1048576.0f) * 200.0f) - 50.0f;

    return true;
}