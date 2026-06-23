Pensando em Engenharia de Firmware , projetei este ecossistema focando em **determinismo, baixa latência e tolerância a falhas**.
Para a comunicação entre o ESP32 DevKit v1 e o Arduino Nano, escolhi a **Serial UART dedicada (HardwareSerial)**. Ela é mais robusta contra ruídos eletromagnéticos de motores do que o I2C e elimina o *overhead* de clock/arbitragem de barramento.
Para mitigar o ruído analógico inerente do ADC do ESP32-C3 e eliminar a trepidação (*jitter*) do joystick, implementei um **Filtro de Média Móvel Exponencial (EMA)** diretamente no Transmissor. É um filtro leve que não adiciona latência perceptível (ao contrário de médias móveis tradicionais) e estabiliza o sinal antes do envio.

## 1. TRANSMISSOR: ESP32-C3 + Joystick Wireless
### Pinagem Sugerida
 * GPIO 0 -> Eixo X do Joystick (ADC1_CH0)
 * GPIO 1 -> Eixo Y do Joystick (ADC1_CH1)
 * *Nota: O ESP32-C3 opera em 3.3V. Alimente o joystick na linha de 3.3V do ESP para evitar leituras saturadas.*

- [Código do Transmissor (ESP32-C3)](./joystick.ino)


## 2. RECEPTOR / CÉREBRO: ESP32 DevKit v1
### Pinagem Sugerida
 * GPIO 16 (RX2) -> Conectado ao TX do Arduino Nano (Opcional, para telemetria reversa)
 * GPIO 17 (TX2) -> Conectado ao RX do Arduino Nano (Crucial)
 * *GND comum entre ESP32 e Arduino Nano é OBRIGATÓRIO.*

- [Código do Receptor (ESP32 DevKit v1)](./core.ino)


## 3. CONTROLADOR DE HARDWARE: Arduino Nano
### Pinagem Sugerida
 * D0 (RX) -> Conectado ao GPIO 17 (TX2) do ESP32 *(Atenção: Use um divisor de tensão de 3.3V no RX do Arduino se necessário, embora em alta velocidade a recepção direta costuma funcionar estável).*
 * D9  -> Sinal do Servo 1 (Rotação da Base)
 * D10 -> Sinal do Servo 2 (Garra)

- [Código do Controlador (Arduino Nano)](./controlador.ino)

### Notas de Engenharia de Campo:
 1. **GND Unificado:** Lembre-se de conectar o pino GND do ESP32 ao pino GND do Arduino Nano e ao GND da fonte dos servos. Sem essa referência comum, a comunicação UART falhará por ruído flutuante.
 2. **Alimentação dos Servos:** Nunca alimente os servomotores diretamente pelas saídas de 5V do Arduino Nano ou 3.3V do ESP32. Utilize uma fonte externa (ex: 5V-6V @ 3A) dedicada para os motores.



