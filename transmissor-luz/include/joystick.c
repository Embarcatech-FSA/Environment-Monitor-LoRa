#include "joystick.h"
#include "hardware/adc.h"


/**
 * @brief Inicializa o ADC e os pinos analógicos do joystick.
 */
void joystick_init() {
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);
}

/**
 * @brief Lê os valores dos eixos X e Y do joystick.
 *
 * @param x Ponteiro para armazenar o valor do eixo X (0-4095).
 * @param y Ponteiro para armazenar o valor do eixo Y (0-4095).
 */

uint16_t read_adc(uint adc_channel) {
    adc_select_input(adc_channel);
    sleep_us(5);
    return adc_read();
}