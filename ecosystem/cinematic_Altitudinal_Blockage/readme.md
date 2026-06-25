# 🪖 Projeto Arm-Tank: Ecossistema Robótico Avançado de Baixa Latência

<p align="center">
  <img src="https://img.shields.io/badge/Status-Em%20Desenvolvimento-orange?style=for-the-badge" alt="Status">
  <img src="https://img.shields.io/badge/Plataforma-Mecatr%C3%B4nica%20V4-blue?style=for-the-badge" alt="Plataforma">
  <img src="https://img.shields.io/badge/Firmware-C%2B%2B%20%2F%20Arduino-darkgreen?style=for-the-badge" alt="Firmware">
</p>

## 📌 Visão Geral
O **Arm-Tank** é um ecossistema robótico modular de alta performance projetado pela **DSBRTI (DestinyServicesBR)** para controle síncrono e em tempo real de uma unidade de tração lagarta integrada a um braço mecânico multipartes (manipulador cinemático). 

Diferente de abordagens maker tradicionais, o firmware foca estritamente em **determinismo temporal, imunidade a ruídos eletromagnéticos e tolerância a falhas**, implementando algoritmos de estabilização geométrica que elevam o projeto ao nível de pesquisa aplicada em automação industrial.

---

## ⚡ Diferenciais de Engenharia

* **Cinemática Inversa com Bloqueio Altitudinal:** Algoritmo dinâmico acoplado que estabiliza a garra em um plano horizontal plano constante. Quando ativado, o movimento de avanço ou recuo do braço recalcula automaticamente as posições lógicas do servo compensador para anular quedas cinemáticas.
* **Latência Sub-20ms Contínua:** Comunicação sem fio peer-to-peer via protocolo nativo **ESP-NOW**, operando em frame compacto sem o *overhead* de pilhas de rede pesadas.
* **Tratamento Digital de Sinal (DSP Local):** Eliminação de *jitter* e ruídos parasitários do ADC do ESP32-C3 através de filtros de **Média Móvel Exponencial (EMA)** independentes por eixo analógico.
* **Máquina de Estados UART Assíncrona:** Comunicação inter-placas robusta via caracteres delimitadores (`<` e `>`) e varredura por tokens (`strtok`), imunidade a perdas de pacotes e isolada de atrasos bloqueantes de CPU.

---

## 🗺️ Topologia e Fluxo de Dados


```
[ ESP32-C3: Transmissor IHM + TFT display]
│
▼  (ESP-NOW Wireless / 66Hz Continuous Payload)
[ ESP32 DevKit v1: Cérebro / Co-Processador Cinemático ]
│
▼  (HardwareSerial UART Dedicada @ 115200 bps)
[ Arduino Nano: Atuador Físico / Ponte H ] ──► [ Servos SG90 + Esteiras ]
```

---

## 🛠️ Infraestrutura de Hardware e Potência

Para suportar as demandas de torque dinâmico dos motores sem gerar ruídos na lógica digital, a infraestrutura física segue especificações estritas de bancada:

* **Atuadores:** Motores micro-servo clássicos **SG90 9g (Azul)** para os eixos do braço e motores DC diferenciais acoplados ao chassi do tanque.
* **Prototipagem de Baixa Resistência:** Utilização de barramentos sólidos de **fios de cobre rígido espessos** (Barramento GND em comum e Barramento 5V em comum), minimizando resistências de contato e quedas de tensão parasitárias típicas de jumpers comuns.
* **Roadmap de Potência:** Engenharia reversa e desenvolvimento em andamento de uma **Fonte Chaveada Regulada de 5V 5A DIY** projetada componente por componente do zero (estágio de chaveamento indutivo e filtragem LC de saída) para suprir com folga os picos de corrente (*stall current*) dos motores sob carga severa.

---

## 📂 Organização do Repositório e Pinagens

Navegue diretamente pelos módulos de firmware através dos links relativos abaixo:

### 🕹️ 1. [Módulo Transmissor](./ecosystem/joystick.ino) (ESP32-C3)
Captura a interface de dois joysticks analógicos e chaves táteis, aplicando filtragem digital EMA antes do envio cíclico estável a ~66Hz.
* `GPIO 0` -> Eixo X do Joystick do Tanque (Direção)
* `GPIO 1` -> Eixo Y do Joystick do Tanque (Velocidade)
* `GPIO 2` -> Eixo X do Joystick do Braço (Rotação da Base)
* `GPIO 3` -> Eixo Y do Joystick do Braço (Alcance/Distância)
* `GPIO 4` -> Botão Push-Button (Subir Cota de Elevação)
* `GPIO 5` -> Botão Push-Button (Descer Cota de Elevação)
* `GPIO 6` -> Botão Push-Button (Alternar Trava de Altura - Modo Toggle)

### 🧠 2. [Módulo Cérebro / Core](./ecosystem/core.ino) (ESP32 DevKit v1)
Gerencia as zonas mortas dos analógicos, armazena em memória a cota do braço e processa a compensação cinemática linear antes de despachar a string formatada para a UART.
* `GPIO 16 (RX2)` -> Conectado ao TX do Arduino Nano (Retorno / Telemetria)
* `GPIO 17 (TX2)` -> Conectado ao RX do Arduino Nano (Fluxo de Comando Crítico)

### ⚙️ 3. [Módulo Atuador / Tank](./ecosystem/tank.ino) (Arduino Nano)
Decodifica o frame serial delimitado em alta velocidade, distribui os pulsos PWM para os três eixos de servos e manipula a potência diferencial da ponte H, sob a proteção de um Watchdog de segurança.
* `D0 (RX)` -> Conectado ao GPIO 17 (TX2) do ESP32 *(Use divisor de tensão para 3.3V)*
* `D9`     -> Sinal do Servo 1 (Rotação da Base)
* `D10`    -> Sinal do Servo 2 (Alcance / Distância)
* `D11`    -> Sinal do Servo 3 (Elevação Compensada)
* `D5`     -> Saída PWM Ponte H (Motor Esquerdo Chassi)
* `D6`     -> Saída PWM Ponte H (Motor Direito Chassi)

---

### ⚠️ Notas de Engenharia de Campo

1. **Malha de Referência (GND Unificado):** É obrigatório interconectar o pino GND de todas as placas e o terminal negativo da fonte externa de potência no barramento de cobre rígido. Sem essa referência comum zero estável, a comunicação UART sofrerá corrupção por ruído flutuante.
2. **Isolamento de Potência:** Nunca alimente os servomotores SG90 ou motores de tração diretamente pelas saídas de pinos regulados do Arduino ou do ESP32. O surto de corrente dinâmico dos motores causa *brownout* (reinicialização por subtensão) imediata nos chips lógicos.

---

## 🤝 Como Contribuir ou Investir

Este projeto faz parte da linha de pesquisa aplicada da **DSBRTI** para 2026, focando em sistemas embarcados acessíveis e algoritmos mecatrônicos de alta eficiência. Estamos abertos a cooperação técnica e suporte financeiro para:
* Otimização avançada de algoritmos trigonométricos de cinemática inversa.
* Fabricação e teste da Placa de Circuito Impresso (PCB) customizada para a Fonte de 5V 5A DIY.

Se você deseja apoiar o desenvolvimento de hardware deste ecossistema *open-source*, você pode fazer uma contribuição direta via **PIX (Chave Aleatória)**:

```text
00020126890014BR.GOV.BCB.PIX0136d28d2849-1e39-4683-924e-aeae136a6eaf0227Obrigado pela contribuicao 5204000053039865802BR5916IVAN MONTIBELLER6008BLUMENAU62290525ZTdjYTU1OTctOGQ5Mi00Y2E5L630436AD

```
Para discussões de parcerias industriais ou suporte comercial, entre em contato através do nosso ecossistema oficial em **dsbrti.qzz.io**.
Developed with ⚙ by **Ivan Montibeller** — DestinyServicesBR © 2018-2026
```

```
