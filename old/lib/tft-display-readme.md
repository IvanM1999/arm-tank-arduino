 Essas telas TFT coloridas de **1,69 polegadas** (geralmente com resolução de 240 \times 280 pixels) usam quase sempre o controlador **ST7789** via comunicação **SPI**.
Como essas telas exigem o envio de muitos dados rapidamente para formar as cores, nós usamos os pinos de **Hardware SPI** do Arduino (que são fixos e muito mais rápidos).
## 1. Bibliotecas Necessárias
Antes de carregar o código, abra o **Gerenciador de Bibliotecas** do Arduino (Sketch -> Incluir Biblioteca -> Gerenciar Bibliotecas) e instale:
 1. **Adafruit ST7735 and ST7789 Library**
 2. **Adafruit GFX Library** (Gerenciador vai pedir para instalar as dependências, clique em "Install All").
## 2. Esquema de Ligação (Para Arduino UNO / Nano)
Se a sua tela não tiver um regulador de tensão embutido na placa (breakout), lembre-se que o chip ST7789 opera em **3.3V**.
| Pino da Tela TFT | Pino no Arduino UNO / Nano | Função |
|---|---|---|
| **VCC** | 3.3V (ou 5V se a placa tiver regulador) | Alimentação |
| **GND** | GND | Terra |
| **SCL / CLK** | D13 | SPI Clock (Fixo) |
| **SDA / DIN** | D11 | SPI MOSI / Dados (Fixo) |
| **RES / RST** | D9 | Reset da Tela (Configurável) |
| **DC / RS** | D8 | Data / Command (Configurável) |
| **CS** | D10 | Chip Select (Configurável) |
| **BLK / LED** | 3.3V ou D7 | Backlight (Luz de fundo) |
## 3. Código de Teste (Diagnóstico e Cores)
Este script limpa a tela, testa o driver aplicando três cores primárias (Vermelho, Verde e Azul) e escreve um texto de teste para garantir que as dimensões da tela (240 \times 280) e a rotação estão corretas.
```cpp
#include <Adafruit_GFX.h>    // Biblioteca gráfica base
#include <Adafruit_ST7789.h> // Biblioteca específica do chip ST7789
#include <Wire.h>
#include <SPI.h>

// Definição dos pinos configuráveis
#define TFT_CS        10
#define TFT_RST        9 
#define TFT_DC         8

// Inicializa o objeto da tela usando Hardware SPI
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(9600);
  Serial.println("--- Teste de Tela TFT ST7789 (1.69') ---");

  // Inicializa a tela com a resolução padrão de 240x280 pixels
  tft.init(240, 280);           
  
  // Define a rotação (pode variar de 0 a 3 dependendo de como quer a tela)
  tft.setRotation(1); 
  
  Serial.println("Tela Inicializada.");
}

void loop() {
  // Teste 1: Preencher com Cor Vermelha
  Serial.println("Preenchendo: Vermelho");
  tft.fillScreen(ST77XX_RED);
  delay(1000);

  // Teste 2: Preencher com Cor Verde
  Serial.println("Preenchendo: Verde");
  tft.fillScreen(ST77XX_GREEN);
  delay(1000);

  // Teste 3: Preencher com Cor Azul
  Serial.println("Preenchendo: Azul");
  tft.fillScreen(ST77XX_BLUE);
  delay(1000);

  // Teste 4: Tela Preta com Texto de Diagnóstico
  tft.fillScreen(ST77XX_BLACK);
  
  tft.setCursor(10, 20);            // Coluna 10, Linha 20
  tft.setTextColor(ST77XX_WHITE);   // Texto Branco
  tft.setTextSize(2);               // Tamanho do texto
  tft.println("SISTEMA OK");

  tft.setCursor(10, 60);
  tft.setTextColor(ST77XX_YELLOW);  // Texto Amarelo
  tft.setTextSize(1);
  tft.println("Teste de Bancada: 1.69 TFT");
  
  tft.setCursor(10, 80);
  tft.setTextColor(ST77XX_CYAN);
  tft.println("Resolucao: 240x280");

  Serial.println("Texto renderizado. Aguardando proximo ciclo...");
  delay(5000); // Aguarda 5 segundos antes de reiniciar o ciclo de cores
}

```
### O que observar durante o teste?
 * **A tela acendeu mas ficou branca/preta?** Verifique os pinos DC e CS. Se eles estiverem invertidos na fiação, o Arduino não consegue enviar comandos de inicialização.
 * **As cores parecerem invertidas (ex: onde devia ser vermelho, fica azul)?** Algumas variações dessas telas baratas usam drivers com a ordem dos pixels invertida (BGR em vez de RGB). Se acontecer com vocês, avise-me e fazemos o ajuste no código!
