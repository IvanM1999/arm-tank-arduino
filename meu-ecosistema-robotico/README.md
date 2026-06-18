# 🔬 Unidade de Laboratório (ESP32 DevKit v1)

Este módulo atua como uma estação científica embarcada responsável pela coleta de dados ambientais e de substrato. Baseado no ecossistema ESP32, ele foi projetado para ler múltiplos sensores simultaneamente através de barramentos digitais e analógicos, preparando as informações para telemetria ou exibição local.

## 📡 Sensores Integrados
1. **Barômetro (BMP280):** Leitura de temperatura, pressão atmosférica e estimativa altimétrica via protocolo I2C.
2. **Higrômetro de Solo Capacitivo v1.2:** Medição de umidade do solo em porcentagem por meio de leitura analógica (ADC).

---

## 🔌 Pinagem e Esquema de Conexões

> ⚠️ **Atenção:** O ESP32 opera com nível lógico de **3.3V**. Certifique-se de alimentar todos os sensores na saída de 3.3V do microcontrolador para evitar danos aos pinos de entrada.

| Sensor / Componente | Pino no Sensor | Pino no ESP32 | Tipo de Sinal / Protocolo |
| :--- | :--- | :--- | :--- |
| **BMP280 (Barômetro)** | VCC | 3.3V | Alimentação |
| **BMP280 (Barômetro)** | GND | GND | Referência (Terra) |
| **BMP280 (Barômetro)** | SCL | **GPIO 22** | I2C Hardware Clock |
| **BMP280 (Barômetro)** | SDA | **GPIO 21** | I2C Hardware Data |
| **Higrômetro Solo** | VCC | 3.3V | Alimentação |
| **Higrômetro Solo** | GND | GND | Referência (Terra) |
| **Higrômetro Solo** | AOUT | **GPIO 34** | Analógico (ADC1_CH6) |

---

## 📚 Bibliotecas Requeridas
Para compilar o código unificado deste laboratório, abra o gerenciador de bibliotecas da IDE do Arduino e instale:
* **Adafruit BMP280 Library** (Instalar todas as dependências sugeridas)

---

## 🛠️ Calibração do Higrômetro
Como o higrômetro de solo é analógico, variações de lote exigem calibração empírica. No código unificado, ajuste as constantes:
* `VALOR_SECO`: Valor bruto lido com o sensor totalmente exposto ao ar.
* `VALOR_MOLHADO`: Valor bruto lido com o sensor mergulhado em água até o limite físico da placa.
