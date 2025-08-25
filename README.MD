# 🌱 Environment Monitor LoRa

Sistema de **monitoramento ambiental para plantas** baseado no **Raspberry Pi Pico W** com comunicação **LoRa** entre duas placas (Transmissor e Receptor).

O **Transmissor** coleta dados de sensores (temperatura, umidade, luminosidade e "gás" simulado por joystick) e os envia via LoRa.
O **Receptor** recebe os dados, exibe no display OLED e fornece feedback visual por LED RGB.

---

## 📡 Funcionalidades

- Coleta de dados ambientais:
  - 🌡️ Temperatura (AHT20)
  - 💧 Umidade (AHT20)
  - 💡 Luminosidade (BH1750)
  - 🟢 Gás (simulado com joystick no ADC)
- Processamento de dados com **Rede Neural MLP** embarcada
- Envio dos dados via **LoRa SX1276/78**
- Exibição em **display OLED SSD1306** no Receptor
- Indicação visual do estado do sistema com **LED RGB**

---

## 📂 Estrutura do Projeto

```
Environment-Monitor-LoRa/
│── transmitter/ # Código do Transmissor (sensores + envio LoRa)
│── receiver/ # Código do Receptor (display + recepção LoRa)
│── include/ # Headers comuns (config, lora, display, etc.)
│── dados.h # Estruturas e variáveis globais
│── README.md # Este arquivo
```

---

## ⚙️ Hardware Necessário

### Transmissor 🌍

- Raspberry Pi Pico W
- Sensor AHT20 (Temperatura/Umidade)
- Sensor BH1750 (Luminosidade)
- Joystick (entrada analógica simulando gás)
- Módulo LoRa SX1276/78 (SPI)
- Display OLED SSD1306 (I2C1)

### Receptor 📺

- Raspberry Pi Pico W
- Módulo LoRa SX1276/78 (SPI)
- Display OLED SSD1306 (I2C1)
- LED RGB

---

## 🚀 Como Rodar

```
1. Clonar o repositóriogit clone https://github.com/seu-usuario/Environment-Monitor-LoRa.git
cd Environment-Monitor-LoRa
2. Compilar e gravar no Pico
O projeto foi desenvolvido em C usando o Pico SDK.
Siga os passos de configuração do Raspberry Pi Pico SDK.
```

Exemplo de build:

```
mkdir build
cd build
cmake ..
make
```

Grave o .uf2 gerado no Pico Transmissor e no Pico Receptor.

## 🧠 Rede Neural MLP


O transmissor aplica um modelo MLP (Multilayer Perceptron) treinado previamente para calcular uma métrica ambiental combinada (environment).


* Entrada (4): Temperatura, Umidade, Gás, Luminosidade
* Oculta (10): Camada intermediária com pesos fixos

* Saída (1): Indicador ambiental normalizado

### 📌 Próximos Passos

* Adicionar suporte a mais sensores (ex: MQ-135 real para gás)
*  Integrar comunicação LoRaWAN

*  Exportar dados via Wi-Fi para nuvem/servidor
