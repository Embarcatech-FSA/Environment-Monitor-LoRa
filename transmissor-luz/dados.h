#ifndef DADOS_H
#define DADOS_H

#include <stdint.h>
#include <stdbool.h>

// Estrutura para conter todos os dados do sistema
typedef struct {
    // Dados dos sensores
    float environment;
    float temperatura;
    float umidade;
    float luminosidade;
    float gas; // << CAMPO ALTERADO: de 'pressao' para 'luminosidade'

    // Estado do LoRa
    bool lora_ok;
    uint32_t pacotes_enviados;

} DadosSistema_t;

#endif // DADOS_H