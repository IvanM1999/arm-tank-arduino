
# 🛠️ Manual de Testes & Bancada Arduino
Este repositório reúne uma coleção de **códigos de teste isolados (testbench)** para validação individual de componentes e periféricos. O objetivo é garantir que cada hardware funcione de forma independente antes de ser integrado ao sistema multi-placas definitivo.
## 📺 1. Tela TFT Colorida 1.69" (Driver ST7789)
Display LCD colorido de alta densidade operando via comunicação SPI por hardware. Exige atenção especial à pinagem estável para evitar artefatos visuais.
### 📚 Bibliotecas Necessárias
 * Adafruit ST7735 and ST7789 Library
 * Adafruit GFX Library
### 🔌 Esquema de Ligação (Hardware SPI)
| Pino da Tela TFT | Pino no Arduino UNO / Nano | Função / Descrição |
|---|---|---|
| **VCC** | 3.3V ou 5V | Alimentação (Verifique se há regulador na placa) |
| **GND** | GND | Terra de referência comum |
| **SCL / CLK** | **D13** | SPI Hardware Clock (Fixo) |
| **SDA / DIN** | **D11** | SPI Hardware MOSI / Dados (Fixo) |
| **RES / RST** | **D9** | Reset Físico da Tela (Configurável) |
| **DC / RS** | **D8** | Data / Command Selection (Configurável) |
| **CS** | **D10** | Chip Select Ativo em LOW (Configurável) |
| **BLK / LED** | 3.3V ou **D7** | Controle de Luz de Fundo (Backlight) |
## 📏 2. Sensor Ultrassônico de Distância (HC-SR04)
Módulo de medição de distância por emissão e recepção de pulsos sônicos. Ideal para leitura precisa de proximidade entre 2cm e 400cm.
### 📚 Bibliotecas Necessárias
 * *Nenhuma* (Usa temporização nativa por meio da função pulseIn()).
### 🔌 Esquema de Ligação
| Pino do HC-SR04 | Pino no Arduino UNO / Nano | Função / Descrição |
|---|---|---|
| **VCC** | 5V | Alimentação de VCC (5V) |
| **Trig** | **D8** | Gatilho para disparo do pulso (Output) |
| **Echo** | **D7** | Retorno do eco sônico (Input) |
| **GND** | GND | Terra de referência comum |
## 🪪 3. Leitor de Cartões e Tags RFID (RC522)
Módulo de autenticação por radiofrequência (13.56\text{ MHz}). Usado para liberação de acessos, chaves eletrônicas e validação de usuários.
### 📚 Bibliotecas Necessárias
 * MFRC522 (por *Community*)
### 🔌 Esquema de Ligação (Hardware SPI)
| Pino do RC522 | Pino no Arduino UNO / Nano | Função / Descrição |
|---|---|---|
| **VCC** | **3.3V** | **⚠️ ATENÇÃO:** Não ligue em 5V para não queimar |
| **RST** | **D9** | Reset do Módulo RFID |
| **GND** | GND | Terra de referência comum |
| **MISO** | **D12** | SPI Hardware Master In Slave Out (Fixo) |
| **MOSI** | **D11** | SPI Hardware Master Out Slave In (Fixo) |
| **SCK** | **D13** | SPI Hardware Serial Clock (Fixo) |
| **SDA / SS / CS** | **D10** | SPI Slave Select (Configurável) |
## 🚨 4. Sensor de Presença Infravermelho (PIR HC-SR501)
Sensor volumétrico passivo para detecção de movimento baseado em variações na radiação infravermelha do ambiente.
### 📚 Bibliotecas Necessárias
 * *Nenhuma* (Leitura digital simples via digitalRead()).
### 🔌 Esquema de Ligação
| Pino do PIR | Pino no Arduino UNO / Nano | Função / Descrição |
|---|---|---|
| **VCC** | 5V | Alimentação Principal (5V) |
| **OUT / DATA** | **D2** | Saída lógica (HIGH se houver movimento) |
| **GND** | GND | Terra de referência comum |
## 💡 Diretrizes Importantes de Bancada
 1. **Barramento Serial:** Todos os códigos desta biblioteca estão pré-configurados para operar na velocidade de **9600 bps**. Certifique-se de configurar o *Monitor Serial* da IDE do Arduino para a mesma velocidade.
 2. **Referência de Terra (GND) Unificado:** Se o sistema utilizar fontes externas diferentes para alimentar displays ou motores, **todos os pinos GND devem estar interconectados** para garantir o mesmo nível lógico de referência.
 3. **Isolamento de Falhas:** Nunca tente misturar códigos no firmware principal antes que cada linha desta tabela tenha validado com sucesso o seu componente correspondente de forma isolada.
