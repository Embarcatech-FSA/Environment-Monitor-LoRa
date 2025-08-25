#ifndef BH1750_H
#define BH1750_H

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include <stdbool.h>

/**
 * @brief Inicializa e liga o sensor BH1750.
 * @param i2c Ponteiro para a instância do barramento I2C (ex: i2c0).
 * @return true se o sensor foi encontrado e inicializado, false caso contrário.
 */
bool bh1750_init(i2c_inst_t* i2c);

/**
 * @brief Lê uma medida de luz ambiente do sensor.
 * Esta função envia o comando de medição e espera pela conversão (~180ms).
 * @param i2c Ponteiro para a instância do barramento I2C.
 * @return A luminosidade em Lux (uint16_t).
 */
uint16_t bh1750_read_lux(i2c_inst_t* i2c);

#endif