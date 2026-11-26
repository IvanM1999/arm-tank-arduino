# 🤖 Braço e Garra Robótica Adaptável (4, 6 ou 8 Eixos) — Arduino Nano

Firmware robusto desenvolvido para **Arduino Nano** (totalmente compatível com compilação via celular pelo aplicativo **ArduinoDroid**). O sistema controla uma garra/braço robótico articulado através de dois joysticks analógicos, possui auto-detecção de configuração de eixos (4, 6 ou 8 servos), gravação de trajetórias em memória não-volátil (EEPROM) e telemetria cinemática em tempo real via Serial.

---

## ✨ Funcionalidades Principais

* **Auto-Detecção de Configuração (4, 6 ou 8 Servos):** Escala automaticamente a quantidade de eixos ativos através de pinos de seleção (*Sense Pins* D11 e D12) ligados ao GND.
* **Controle Suave por 2 Joysticks Analógicos:** Inclui filtro de zona morta (*deadzone*) para eliminar tremores causados por desgaste nos potenciômetros e controle proporcional de velocidade.
* **Alternância de Camadas (Layer Switch):** Permite controlar até 8 servos utilizando apenas 2 joysticks (4 eixos por vez) através de um botão seletor.
* **Cinemática Direta e Domo de Operação:** Calcula em tempo real a altura atual da garra (`Z` em mm), o raio de alcance atual e o limite máximo do domo de operação com base na trigonometria dos elos.
* **Memória Não-Volátil (EEPROM):** Grava até 15 posições completas (waypoints) que permanecem salvas mesmo após desligar a energia, com reprodução interpolada e suave.
* **Soft-Start e Proteção no Reboot:** Ao ligar ou reiniciar, inicializa os motores sequencialmente na posição neutra de **90°** (entre 0° e 180°), evitando picos de corrente e movimentos bruscos.
* **Telemetria Serial Completa:** Envia continuamente a especificação do servo (modelo, ângulo, capacidade de carga), altura, raio no domo e o ângulo exato de cada motor.

---

## 🔌 Esquema de Ligação (Pinout - Arduino Nano)

### Servos Motores
| Eixo / Articulação | Pino Arduino | Disponibilidade |
| :--- | :---: | :--- |
| **Servo 1 (Base)** | `D2` | Sempre ativo (Modo 4, 6 e 8) |
| **Servo 2 (Ombro)** | `D3` | Sempre ativo (Usado no cálculo de altura) |
| **Servo 3 (Cotovelo)** | `D4` | Sempre ativo (Usado no cálculo de altura) |
| **Servo 4 (Garra)** | `D5` | Sempre ativo (Modo 4, 6 e 8) |
| **Servo 5 (Punho Pitch)** | `D6` | Ativo nos modos **6** e **8** servos |
| **Servo 6 (Punho Roll)** | `D7` | Ativo nos modos **6** e **8** servos |
| **Servo 7 (Auxiliar 1)** | `D8` | Ativo apenas no modo **8** servos |
| **Servo 8 (Auxiliar 2)** | `D9` | Ativo apenas no modo **8** servos |

### Seleção de Modo (Hardware Sense)
| Configuração Desejada | Conexão Necessária |
| :--- | :--- |
| **4 Servos (Padrão)** | Deixar `D11` e `D12` desconectados |
| **6 Servos** | Ligar o pino **`D11`** ao **`GND`** |
| **8 Servos** | Ligar o pino **`D12`** ao **`GND`** |

### Controles e Botões
| Componente | Pino Arduino | Função |
| :--- | :---: | :--- |
| **Joystick 1 (X / Y)** | `A0` / `A1` | Controla S1/S2 (ou S5/S6 na camada 2) |
| **Joystick 2 (X / Y)** | `A2` / `A3` | Controla S3/S4 (ou S7/S8 na camada 2) |
| **Botão SAVE** | `A4` (+ GND) | Toque curto: grava ponto / Segurar 2s: limpa memória |
| **Botão PLAY** | `A5` (+ GND) | Executa a sequência gravada na EEPROM |
| **Botão LAYER** | `D10` (+ GND) | Alterna joysticks entre Servos 1-4 e Servos 5-8 |

> ⚠️ **AVISO IMPORTANTE DE ALIMENTAÇÃO:** Nunca alimente 4 a 8 servos diretamente pelo pino `5V` do Arduino Nano. Utilize uma fonte externa dedicada de **5V a 6V (3A a 5A)** para o positivo (VCC) dos servos e conecte o negativo (**GND**) da fonte ao **GND** do Arduino Nano.

---

## ⚙️ Calibração Mecânica

Antes de compilar, ajuste as constantes no topo do arquivo `.ino` de acordo com as medidas físicas da sua estrutura e modelo de motor:

```cpp
const char* SERVO_MODELO = "SG90_Micro";
const int SERVO_ANG_MIN = 0;
const int SERVO_ANG_MAX = 180;
const int SERVO_FORCA_GRAMAS = 1800; // Ex: 1800g (1.8kg.cm) ou 500g

const float L1_BASE_ALTURA = 50.0;  // Altura da base até o eixo do ombro (mm)
const float L2_BRACO = 100.0;       // Comprimento entre ombro e cotovelo (mm)
const float L3_ANTEBRACO = 100.0;   // Comprimento entre cotovelo e ponta da garra (mm)
