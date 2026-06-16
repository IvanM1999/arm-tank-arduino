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