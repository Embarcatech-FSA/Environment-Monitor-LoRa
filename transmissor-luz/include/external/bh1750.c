#include "bh1750.h"

// Comandos do BH1750
const uint8_t BH1750_CMD_POWER_ON = 0x01;
const uint8_t BH1750_CMD_CONT_HRES_MODE = 0x10; // Modo de alta resolução contínuo (1 lux)

// Função privada para simplificar a escrita no I2C
static int _i2c_write_byte(i2c_inst_t* i2c, uint8_t byte) {
    return i2c_write_blocking(i2c, 0x23, &byte, 1, false);
}

bool bh1750_init(i2c_inst_t* i2c) {
    // Para verificar se o sensor está presente, tentamos ligá-lo.
    // Se a escrita falhar (retornar erro), o sensor não está conectado.
    int result = _i2c_write_byte(i2c, BH1750_CMD_POWER_ON);
    sleep_ms(10); // Pequena pausa após power on

    if (result < 1) {
        return false; // Erro na comunicação I2C
    }
    return true;
}

uint16_t bh1750_read_lux(i2c_inst_t* i2c) {
    // 1. Envia o comando para iniciar uma medição em modo de alta resolução
    _i2c_write_byte(i2c, BH1750_CMD_CONT_HRES_MODE);

    // 2. Aguarda o tempo de medição (datasheet recomenda >120ms, 180ms é seguro)
    sleep_ms(180);

    uint8_t buffer[2];
    // 3. Lê os 2 bytes do resultado
    int bytes_read = i2c_read_blocking(i2c, 0x23, buffer, 2, false);
    if (bytes_read < 2) {
        return 0; // Retorna 0 em caso de erro de leitura
    }

    // 4. Combina os bytes e aplica o fator de conversão (1.2 para H-Res Mode)
    uint16_t raw_lux = ((uint16_t)buffer[0] << 8) | buffer[1];
    return (uint16_t)(raw_lux / 1.2f);
}