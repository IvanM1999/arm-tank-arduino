#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Adafruit_GFX.h>    // Biblioteca gráfica base
#include <Adafruit_ST7789.h> // Biblioteca específica para telas IPS ST7789

// --- Configuração dos Pinos ---
// NRF24L01 no ESP-C3
#define NRF_CE   2
#define NRF_CSN  3

// Tela TFT 1.69"
#define TFT_CS   9
#define TFT_DC   8
#define TFT_RST  7

// Botões de Direção (Ultra Baixo Custo: Botões simples)
#define BTN_FRENTE   10
#define BTN_TRAS     18
#define BTN_ESQUERDA 19
#define BTN_DIREITA  20

// --- Inicialização dos Objetos ---
RF24 radio(NRF_CE, NRF_CSN);
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

const byte endereco[6] = "TANK1";

// Estrutura de dados idêntica à do Arduino Nano (Robô)
struct PacoteControle {
  char direcao;     // 'F'=Frente, 'T'=Trás, 'E'=Esquerda, 'D'=Direita, 'P'=Parado
  int anguloBase;   // Ângulo simulado do braço
  int anguloGarra;  // Ângulo simulado da garra
};

PacoteControle meusComandos;
char ultimaDirecaoExibida = ' ';

void setup() {
  Serial.begin(115200);
  
  // Configuração dos Pinos dos Botões (Pull-up interno: não precisa de resistor externo)
  pinMode(BTN_FRENTE, INPUT_PULLUP);
  pinMode(BTN_TRAS, INPUT_PULLUP);
  pinMode(BTN_ESQUERDA, INPUT_PULLUP);
  pinMode(BTN_DIREITA, INPUT_PULLUP);

  // Inicializa a Tela TFT Colorida
  tft.init(240, 280); // Resolução padrão aproximada para telas de 1.69"
  tft.setRotation(1); // Modo paisagem (horizontal)
  tft.fillScreen(ST77XX_BLACK);
  
  // Desenha o Painel Inicial na Tela
  tft.setTextColor(ST77XX_BLUE);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("ECOSSISTEMA ROBOTICO");
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);
  tft.setCursor(10, 35);
  tft.println("Controle Remoto ESP-C3");
  
  tft.drawRect(5, 55, 230, 80, ST77XX_YELLOW);
  tft.setCursor(15, 65);
  tft.setTextColor(ST77XX_YELLOW);
  tft.println("STATUS DO LINK NRF24:");

  // Inicializa o Rádio NRF24L01
  if (!radio.begin()) {
    tft.setCursor(15, 85);
    tft.setTextColor(ST77XX_RED);
    tft.println("ERRO hardware NRF!");
    Serial.println("Erro no rádio.");
    while (1);
  }

  radio.openWritingPipe(endereco);
  radio.setPALevel(RF24_PA_MAX); // Força potência máxima (PA+LNA ativo) para longo alcance
  radio.setDataRate(RF24_1MBPS);
  radio.stopListening();         // Define o controle como TRANSMISSOR

  tft.setCursor(15, 85);
  tft.setTextColor(ST77XX_GREEN);
  tft.println("Radio ON - Transmitindo");
  
  // Valores padrão para os servos da garra (posição centralizada)
  meusComandos.anguloBase = 90;
  meusComandos.anguloGarra = 90;
}

void loop() {
  // Leitura dos botões físicos (Lógica invertida devido ao INPUT_PULLUP: pressionado = LOW)
  if (digitalRead(BTN_FRENTE) == LOW) {
    meusComandos.direcao = 'F';
  } else if (digitalRead(BTN_TRAS) == LOW) {
    meusComandos.direcao = 'T';
  } else if (digitalRead(BTN_ESQUERDA) == LOW) {
    meusComandos.direcao = 'E';
  } else if (digitalRead(BTN_DIREITA) == LOW) {
    meusComandos.direcao = 'D';
  } else {
    meusComandos.direcao = 'P'; // Parado se nenhum botão for pressionado
  }

  // Envia o pacote de dados completo via rádio para o Arduino Nano
  bool sucessoEnvio = radio.write(&meusComandos, sizeof(PacoteControle));

  // Atualiza a Tela IPS apenas se a direção mudar (evita efeito de oscilação/piscar da tela)
  if (meusComandos.direcao != ultimaDirecaoExibida) {
    // Apaga a área anterior da tela
    tft.fillRect(15, 105, 200, 20, ST77XX_BLACK); 
    tft.setCursor(15, 105);
    
    if (sucessoEnvio) {
      tft.setTextColor(ST77XX_GREEN);
      tft.print("Comando Enviado: "); tft.println(meusComandos.direcao);
    } else {
      tft.setTextColor(ST77XX_RED);
      tft.println("Falha no Link (Sem Sinal)");
    }
    ultimaDirecaoExibida = meusComandos.direcao;
  }

  delay(30); // Envia comandos aproximadamente 33 vezes por segundo (baixa latência)
}
