#ifndef AHT20_H
#define AHT20_H

#include <stdbool.h>
#include <stdint.h>

// Inicializa o sensor AHT20
bool aht20_init();

// Lê os valores de temperatura e umidade do sensor.
// temperature em °C
// humidity em %RH
bool aht20_read_data(float *temperature, float *humidity);

#endif // AHT20_H