/**
 * @file Controle_Central_TFT_NRF24.ino
 * @brief Gerenciador do Controle Remoto: Recebe via NRF24L01 e exibe na Tela TFT 1.69"
 * @board ESP32-C3
 * @layer Gerenciamento Central
 */

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <RF24.h>

// --- Definições dos Pinos da Tela TFT ---
#define TFT_CS         8
#define TFT_DC         2
#define TFT_RST        9

// --- Definições dos Pinos do Rádio NRF24L01 ---
#define RADIO_CE       7
#define RADIO_CSN      3

// Inicialização dos Objetos (Compartilhando o Barramento SPI nativo)
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
RF24 radio(RADIO_CE, RADIO_CSN);

// Endereço do canal de comunicação (Deve ser o mesmo da Unidade de Laboratório)
const byte endereco[6] = "LAB01";

// Estrutura exata dos dados transmitidos pelo Laboratório
struct DadosLaboratorio {
  float temperatura;
  float pressao;
  float altitude;
  int umidadeSolo;
};

DadosLaboratorio dadosRecebidos;
unsigned long ultimoPacoteTempo = 0;
bool alternarPisca = false;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("\n=== INICIALIZANDO GERENCIADOR CENTRAL CONTROLE ===");

  // 1. Inicializa Tela TFT
  tft.init(240, 280);
  tft.setRotation(1); // Modo Paisagem
  tft.fillScreen(ST77XX_BLACK);
  desenharInterfaceBase();

  // 2. Inicializa Rádio NRF24L01
  if (!radio.begin()) {
    tft.setCursor(10, 100);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(2);
    tft.println("ERRO HARDWARE NRF24");
    Serial.println("❌ Erro crítico: Módulo NRF24 não respondeu!");
    while (1);
  }

  radio.openReadingPipe(1, endereco);
  radio.setPALevel(RF24_PA_LOW);  // Baixa potência para bancada
  radio.setDataRate(RF24_1MBPS);
  radio.startListening();          // Modo Receptor Ativo

  tft.setCursor(10, 250);
  tft.setTextColor(ST77XX_GREEN);
  tft.setTextSize(1);
  tft.println("SISTEMA OK - AGUARDANDO TELEMETRIA");
  Serial.println("✅ Controle Remoto pronto e escutando...");
}

void loop() {
  // Verifica se chegou algum pacote de telemetria pelo rádio
  if (radio.available()) {
    radio.read(&dadosRecebidos, sizeof(dadosRecebidos));
    ultimoPacoteTempo = millis(); // Atualiza timestamp do último pacote
    
    // Atualiza os valores numéricos na tela gráfica
    atualizarDadosTela();
  }

  // Monitor de desconexão (Se passar de 5 segundos sem receber nada, alerta)
  if (millis() - ultimoPacoteTempo > 5000) {
    tft.fillRect(190, 12, 80, 16, ST77XX_BLACK);
    tft.setCursor(190, 12);
    tft.setTextColor(ST77XX_RED);
    tft.setTextSize(1);
    tft.print("OFFLINE");
  } else {
    // Efeito visual de "Batimento Cardíaco" (Heartbeat) no canto da tela indicando link vivo
    tft.fillRect(260, 12, 10, 10, alternarPisca ? ST77XX_GREEN : ST77XX_BLACK);
    alternarPisca = !alternarPisca;
    delay(200); 
  }
}

/**
 * @brief Desenha o layout estático da tela (Executado apenas uma vez no setup)
 */
void desenharInterfaceBase() {
  tft.fillRect(0, 0, 280, 35, tft.color565(30, 40, 60)); // Barra de título superior
  
  tft.setCursor(10, 10);
  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.print("TELEMETRIA ROBOTICA");

  // Rótulos fixos para evitar que a tela pisque ao reescrevê-los
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  
  tft.setCursor(10, 60);  tft.print("Temp. Ar   :");
  tft.setCursor(10, 100); tft.print("Pressao    :");
  tft.setCursor(10, 140); tft.print("Altitude   :");
  tft.setCursor(10, 180); tft.print("Umidade Sol:");
  
  // Linha decorativa inferior
  tft.drawFastHLine(0, 230, 280, tft.color565(80, 80, 80));
}

/**
 * @brief Atualiza apenas as áreas dos números com os novos dados recebidos
 */
void actualizarDadosTela() {
  tft.setTextSize(2);
  tft.setTextColor(ST77XX_YELLOW, ST77XX_BLACK); // O segundo parâmetro desenha o fundo preto automaticamente, removendo o rastro do número anterior

  // Atualiza campo de Temperatura
  tft.setCursor(160, 60);
  tft.print(dadosRecebidos.temperatura, 1);
  tft.print(" C ");

  // Atualiza campo de Pressão
  tft.setCursor(160, 100);
  tft.print(dadosRecebidos.pressao, 0);
  tft.print(" hPa");

  // Atualiza campo de Altitude
  tft.setCursor(160, 140);
  tft.print(dadosRecebidos.altitude, 0);
  tft.print(" m  ");

  // Atualiza campo de Umidade do Solo
  tft.setCursor(160, 180);
  tft.print(dadosRecebidos.umidadeSolo);
  tft.print(" %   ");

  // Atualiza indicador de status do link superior
  tft.fillRect(190, 12, 65, 16, tft.color565(30, 40, 60));
  tft.setCursor(190, 12);
  tft.setTextColor(ST77XX_GREEN, tft.color565(30, 40, 60));
  tft.setTextSize(1);
  tft.print("ONLINE");
}
