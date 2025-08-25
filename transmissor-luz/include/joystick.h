#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "pico/stdlib.h"

//Inicializa o subsistema ADC e configura os pinos GPIO conectados ao joystick como entradas analógicas.
void joystick_init();

// Lê os valores atuais dos eixos X e Y do joystick.
uint16_t read_adc(uint adc_channel);

#endif 