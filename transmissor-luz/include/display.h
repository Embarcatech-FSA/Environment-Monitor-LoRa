#ifndef DISPLAY_H
#define DISPLAY_H

#include "dados.h" // Inclui a estrutura de estado 'DadosSistema_t'
#include "lib/ssd1306/ssd1306.h"
#include "lib/ssd1306/font.h" 

void display_init(ssd1306_t *ssd);

void display_startup_screen(ssd1306_t *ssd);

void display_update_screen(ssd1306_t *ssd, DadosSistema_t *dados);

void display_error_screen(ssd1306_t *ssd, const char *message);

#endif // DISPLAY_H