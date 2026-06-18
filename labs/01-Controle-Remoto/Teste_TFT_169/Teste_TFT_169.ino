/**
 * @file Teste_TFT_169.ino
 * @brief Código de teste isolado para Tela TFT 1.69" (ST7789) no ESP32-C3
 * @layer Controle Remoto
 */

#include <Adafruit_GFX.h>    // Biblioteca gráfica base da Adafruit
#include <Adafruit_ST7789.h> // Driver específico para o controlador ST7789
#include <SPI.h>

// Definição dos pinos de controle configuráveis para o ESP32-C3
#define TFT_CS         8  // Chip Select (Pino de seleção do chip)
#define TFT_RST        9  // Reset do display
#define TFT_DC         2  // Data/Command (Seleção de comando ou dados)

// Nota sobre os pinos fixos de Hardware SPI no ESP32-C3:
// MOSI (SDA) -> GPIO 6
// SCK (SCL)  -> GPIO 4

// Inicializa o objeto do display passando os pinos de controle
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("[TFT] Inicializando teste de tela...");

  // Inicializa o display com a resolução exata de 240x280 pixels
  tft.init(240, 280);           
  
  // Define a rotação da tela (1 ou 3 para modo paisagem / landscape)
  tft.setRotation(1); 
  
  // Limpa a tela preenchendo com preto
  tft.fillScreen(ST77XX_BLACK);
  Serial.println("[TFT] Tela inicializada com sucesso.");
}

void loop() {
  // --- TESTE 1: Varredura de Cores Sólidas ---
  Serial.println("[TFT] Ciclo de Cores...");
  
  tft.fillScreen(ST77XX_RED);
  delay(1000);
  
  tft.fillScreen(ST77XX_GREEN);
  delay(1000);
  
  tft.fillScreen(ST77XX_BLUE);
  delay(1000);

  // --- TESTE 2: Renderização de Texto e Interface Visual ---
  tft.fillScreen(ST77XX_BLACK);
  
  // Título do Bloco
  tft.setCursor(10, 15);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2); // Texto médio
  tft.println("CONTROLE REMOTO");

  // Linha divisória estética (X_inicio, Y_inicio, X_fim, Y_fim, Cor)
  tft.drawLine(10, 35, 270, 35, ST77XX_WHITE);

  // Status simulado do Link de Rádio
  tft.setCursor(10, 55);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(1); // Texto pequeno
  tft.print("STATUS LINK: ");
  tft.setTextColor(ST77XX_GREEN);
  tft.println("CONECTADO");

  // Dados de Telemetria simulados
  tft.setCursor(10, 80);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("Bateria Tanque: ");
  tft.setTextColor(ST77XX_MAGENTA);
  tft.println("12.6V");

  tft.setCursor(10, 100);
  tft.setTextColor(ST77XX_WHITE);
  tft.print("Sinal NRF24: ");
  tft.setTextColor(ST77XX_CYAN);
  tft.println("-45 dBm");

  Serial.println("[TFT] Interface renderizada na tela. Aguardando 6 segundos.");
  delay(6000); 
}
