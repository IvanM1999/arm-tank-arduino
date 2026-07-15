#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_now.h>
#include <WiFi.h>

// ---- MAPEAMENTO DE PINOS - ESP32-C3 MINI ----
#define JOY1_X     0
#define JOY1_Y     1
#define JOY2_X     2
#define JOY2_Y     3
#define TX_CABO    6  // Comunicação serial de backup (fio)
#define RX_CABO    7
#define TECLADO_A  8  // Escada de resistores (Múltiplas funções)
#define BACKLIGHT 18  // Controle de iluminação do display (+ / -)
#define BTN_PANICO 19 // Botão pânico físico direto (Segurança máxima)

// ---- CONFIGURAÇÃO DO DISPLAY LOCAL ----
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 displayLocal(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---- ESTRUTURA DE DADOS DE TRANSMISSÃO ----
typedef struct struct_mensagem {
    int j1_x; int j1_y;
    int j2_x; int j2_y;
    byte botoes; // Estado dos botões decodificados por resistores
    bool panico;
} struct_mensagem;

struct_mensagem dadosControle;
uint8_t macTanque[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}; // Substituir pelo MAC real do DevKit

// Variáveis de Filtro de Estabilização (EIR)
float fJ1X = 2048, fJ1Y = 2048, fJ2X = 2048, fJ2Y = 2048;
const float alfa = 0.25; 

// Sistema de monitoramento do Backlight
int nivelBrilho = 128;

void setup() {
    Serial.begin(115200);
    Serial1.begin(115200, SERIAL_8N1, RX_CABO, TX_CABO); // Comunicação via hardware (cabo)
    
    pinMode(BTN_PANICO, INPUT_PULLUP);
    pinMode(BACKLIGHT, OUTPUT);
    analogWrite(BACKLIGHT, nivelBrilho);

    // Inicializa I2C nos pinos internos padrão do C3 Mini
    Wire.begin(4, 5); 
    if(!displayLocal.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Erro display OLED local!");
    }
    displayLocal.clearDisplay();
    displayLocal.setTextSize(1);
    displayLocal.setTextColor(WHITE);

    // Inicialização da comunicação Wireless (ESP-NOW)
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
        Serial.println("Erro ao inicializar ESP-NOW");
    }

    // Registra o receptor (Tanque DevKit)
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, macTanque, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);
}

void loop() {
    // 1. Leituras Analógicas Suavizadas (Evita flutuações e comandos brutos)
    fJ1X = (analogRead(JOY1_X) * alfa) + (fJ1X * (1.0 - alfa));
    fJ1Y = (analogRead(JOY1_Y) * alfa) + (fJ1Y * (1.0 - alfa));
    fJ2X = (analogRead(JOY2_X) * alfa) + (fJ2X * (1.0 - alfa));
    fJ2Y = (analogRead(JOY2_Y) * alfa) + (fJ2Y * (1.0 - alfa));

    dadosControle.j1_x = fJ1X - 2048;
    dadosControle.j1_y = fJ1Y - 2048;
    dadosControle.j2_x = fJ2X - 2048;
    dadosControle.j2_y = fJ2Y - 2048;

    // Zona Morta
    if(abs(dadosControle.j1_x) < 150) dadosControle.j1_x = 0;
    if(abs(dadosControle.j1_y) < 150) dadosControle.j1_y = 0;
    if(abs(dadosControle.j2_x) < 150) dadosControle.j2_x = 0;
    if(abs(dadosControle.j2_y) < 150) dadosControle.j2_y = 0;

    // 2. Decodificação do Teclado de Resistores (Pino 8)
    int valorTeclado = analogRead(TECLADO_A);
    dadosControle.botoes = 0; // Reseta estados
    if(valorTeclado > 500 && valorTeclado < 1000)   dadosControle.botoes = 1; // Seta Cima / Abrir Garra
    if(valorTeclado > 1000 && valorTeclado < 1500)  dadosControle.botoes = 2; // Seta Baixo / Fechar Garra
    if(valorTeclado > 1500 && valorTeclado < 2500)  dadosControle.botoes = 3; // Menu / Liga Display

    // 3. Verificação do Botão de Pânico Físico (Pino 19)
    dadosControle.panico = (digitalRead(BTN_PANICO) == LOW);

    // Ajuste dinâmico de brilho via hardware auxiliar (Pino 18)
    if(digitalRead(18) == LOW) {
        nivelBrilho = (nivelBrilho + 32) > 255 ? 32 : nivelBrilho + 32;
        analogWrite(BACKLIGHT, nivelBrilho);
        delay(200);
    }

    // 4. Envio Simultâneo (Wireless + Backup Físico por Cabo)
    esp_now_send(macTanque, (uint8_t *) &dadosControle, sizeof(dadosControle));
    
    Serial1.print("<");
    Serial1.printf("%d,%d,%d,%d,%d,%d", dadosControle.j1_x, dadosControle.j1_y, dadosControle.j2_x, dadosControle.j2_y, dadosControle.botoes, dadosControle.panico);
    Serial1.println(">");

    // Atualização do Display OLED Local do Controle
    displayLocal.clearDisplay();
    displayLocal.setCursor(0,0);
    displayLocal.printf("Ctrl Status: Ativo\nPanico: %s\nJ1: %d, %d", dadosControle.panico ? "SIM" : "NAO", dadosControle.j1_x, dadosControle.j1_y);
    displayLocal.display();

    delay(30); // Ciclo estável de transmissão
}


