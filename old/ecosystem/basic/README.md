# Ecossistema Robótico de Baixa Latência para Controle de Garra

Pensando em Engenharia de Firmware, este ecossistema foi projetado focando em **determinismo, baixa latência e tolerância a falhas**.

* **Comunicação Inter-Placas:** Para a conexão entre o ESP32 DevKit v1 e o Arduino Nano, a escolha técnica foi a **Serial UART dedicada (HardwareSerial)**. Ela é significativamente mais robusta contra ruídos eletromagnéticos de motores do que o I2C e elimina o *overhead* de clock/arbitragem de barramento.
* **Tratamento de Sinal:** Para mitigar o ruído analógico inerente do ADC do ESP32-C3 e eliminar a trepidação (*jitter*) do joystick, foi implementado um **Filtro de Média Móvel Exponencial (EMA)** diretamente no Transmissor. É um filtro leve que não adiciona latência perceptível (ao contrário de médias móveis tradicionais) e estabiliza o sinal antes do envio.

---

## 🛠️ Infraestrutura de Hardware e Potência

Para suportar a dinâmica dos motores sem gerar ruídos na lógica digital, a infraestrutura física segue especificações estritas de bancada:

* **Atuadores:** Motores micro-servo clássicos **SG90 9g (Azul)**. Embora pequenos, os picos de corrente de partida (*stall current*) de múltiplos servos podem derrubar o barramento lógico se mal dimensionados.
* **Prototipagem de Baixa Resistência:** Utilização de matrizes de contato (protoboards) com linhas de alimentação unificadas por **fios de cobre rígido espessos**. O uso de cobre rígido cria barramentos sólidos (Barramento GND em comum e Barramento 5V em comum), minimizando a resistência de contato e quedas de tensão parasitárias comuns em jumpers do tipo Dupont.
* **Alimentação Lógica:** Fontes estáveis de **USB 5V** dedicadas individualmente para alimentar o ecossistema de microcontroladores.
* **Planejamento de Potência Futuro (Roadmap):** Projeto em desenvolvimento de uma **Fonte Chaveada Regulada de 5V 5A DIY**, projetada componente por componente do zero (filtro de entrada, estágio de chaveamento indutivo e filtragem LC de saída) para substituir as fontes USB e suprir com folga a corrente dinâmica dos servos sob carga severa.

---

## 1. TRANSMISSOR: ESP32-C3 + Joystick Wireless

### Pinagem Sugerida
* `GPIO 0` -> Eixo X do Joystick (ADC1_CH0)
* `GPIO 1` -> Eixo Y do Joystick (ADC1_CH1)
* *Nota: O ESP32-C3 opera em 3.3V. Alimente o joystick na linha de 3.3V do ESP para evitar leituras saturadas e danos ao componente.*

💾 **[Código do Transmissor (ESP32-C3)](./joystick.ino)**

---

## 2. RECEPTOR / CÉREBRO: ESP32 DevKit v1

### Pinagem Sugerida
* `GPIO 16 (RX2)` -> Conectado ao TX do Arduino Nano (Opcional, para telemetria reversa)
* `GPIO 17 (TX2)` -> Conectado ao RX do Arduino Nano (Crucial)
* *GND comum entre ESP32 e Arduino Nano é OBRIGATÓRIO.*

💾 **[Código do Receptor (ESP32 DevKit v1)](./core.ino)**

---

## 3. CONTROLADOR DE HARDWARE: Arduino Nano
### Pinagem Sugerida
* `D0 (RX)` -> Conectado ao GPIO 17 (TX2) do ESP32 *(Atenção: Use um divisor de tensão simples ou conversor de nível lógico de 3.3V no RX do Arduino para proteção de barramento, embora em alta velocidade a recepção direta costuma operar de forma estável).*
* `D9`  -> Sinal do Servo 1 (Rotação da Base)
* `D10` -> Sinal do Servo 2 (Abertura/Fechamento da Garra)

💾 **[Código do Controlador (Arduino Nano)](./tank.ino)**

---

### ⚠️ Notas de Engenharia de Campo

1. **GND Unificado (Malha de Referência):** Lembre-se de interconectar o pino GND do ESP32, o pino GND do Arduino Nano e o terminal negativo (GND) da fonte dos servos utilizando o barramento de cobre rígido. Sem essa referência comum zero, os limiares de tensão da comunicação UART flutuam, gerando corrupção de pacotes.
2. **Isolamento de Potência dos Servos:** Nunca alimente os servomotores SG90 diretamente pelas saídas de pinos de 5V do Arduino Nano ou 3.3V do ESP32. O pico de corrente dos motores causa *brownout* (reinicialização por subtensão) instantâneo nos chips lógicos. Sempre utilize o barramento de 5V dedicado alimentado externamente. 
