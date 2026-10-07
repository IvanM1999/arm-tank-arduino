#include <esp_now.h>
#include <WiFi.h>

// Endereço MAC do ESP32 Receptor (Substitua pelo MAC real do seu Receptor)
uint8_t receiverAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

// Definição dos Pinos de Entrada (Hardware)
const int PIN_TANK_X  = 0;
const int PIN_TANK_Y  = 1;
const int PIN_ARM_X   = 2;
const int PIN_ARM_Y   = 3;
const int PIN_BTN_UP  = 4;
const int PIN_BTN_DWN = 5;
const int PIN_BTN_MODE = 6;

// Coeficiente do Filtro EMA (0.0 a 1.0) - Estabiliza o jitter do ADC do ESP32-C3
const float EMA_ALPHA = 0.2;
float fTankX = 2048.0;
float fTankY = 2048.0;
float fArmX  = 2048.0;
float fArmY  = 2048.0;

// Struct Expandida e Otimizada para o Ecossistema Completo (7 Bytes de Payload)
typedef struct struct_message {
    int8_t tankX;       // Direção do chassi (-100 a 100)
    int8_t tankY;       // Velocidade do chassi (-100 a 100)
    int8_t armX;        // Rotação da base (-100 a 100)
    int8_t armY;        // Alcance/Distância (-100 a 100)
    int8_t btnElevUp;   // Comando para subir elevação (0 ou 1)
    int8_t btnElevDown; // Comando para descer elevação (0 ou 1)
    int8_t btnMode;     // Estado interno da trava de altura (0 ou 1)
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

// Callback de monitoramento de envio do ESP-NOW
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Retorno silencioso focado em desempenho e redução de overhead na CPU
}

void setup() {
    Serial.begin(115200);
    
    // Configuração das portas digitais com resistores de Pull-up internos ativos
    pinMode(PIN_BTN_UP, INPUT_PULLUP);
    pinMode(PIN_BTN_DWN, INPUT_PULLUP);
    pinMode(PIN_BTN_MODE, INPUT_PULLUP);
    
    // Configura o subsistema de RF
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // Desativa o gerenciamento de energia para garantir latência sub-20ms

    if (esp_now_init() != ESP_OK) {
        Serial.println("Erro ao inicializar ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);
    
    // Configuração e acoplamento do nó receptor
    memcpy(peerInfo.peer_addr, receiverAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Falha ao adicionar Peer");
        return;
    }
    
    // Inicializa a flag de modo como desativada (0)
    myData.btnMode = 0;
}

void loop() {
    // 1. Leituras Analógicas Brutas (0 - 4095) dos Joysticks
    int rawTankX = analogRead(PIN_TANK_X);
    int rawTankY = analogRead(PIN_TANK_Y);
    int rawArmX  = analogRead(PIN_ARM_X);
    int rawArmY  = analogRead(PIN_ARM_Y);

    // 2. Processamento dos Filtros Digitais Independentes (EMA) contra ruído elétrico
    fTankX = (EMA_ALPHA * rawTankX) + ((1.0 - EMA_ALPHA) * fTankX);
    fTankY = (EMA_ALPHA * rawTankY) + ((1.0 - EMA_ALPHA) * fTankY);
    fArmX  = (EMA_ALPHA * rawArmX)  + ((1.0 - EMA_ALPHA) * fArmX);
    fArmY  = (EMA_ALPHA * rawArmY)  + ((1.0 - EMA_ALPHA) * fArmY);

    // 3. Normalização e Mapeamento Cinematográfico para Banda Estrita (int8_t)
    myData.tankX = map((int)fTankX, 0, 4095, -100, 100);
    myData.tankY = map((int)fTankY, 0, 4095, -100, 100);
    myData.armX  = map((int)fArmX,  0, 4095, -100, 100);
    myData.armY  = map((int)fArmY,  0, 4095, -100, 100);

    // 4. Varredura Lógica dos Botões de Elevação (Lógica Invertida devido ao Pull-Up)
    myData.btnElevUp   = !digitalRead(PIN_BTN_UP);
    myData.btnElevDown = !digitalRead(PIN_BTN_DWN);

    // 5. Máquina de Estado de Transição Simples (Toggle) para Alternância do Modo de Altura
    static bool lastButtonState = false;
    bool currentButtonState = !digitalRead(PIN_BTN_MODE);
    
    if (currentButtonState && !lastButtonState) {
        if (myData.btnMode == 0) {
            myData.btnMode = 1;  // Ativa a trava de plano altitudinal plano
        } else {
            myData.btnMode = 0;  // Libera para modo de controle direto
        }
        delay(50); // Debounce de hardware por software para evitar repiques elétricos
    }
    lastButtonState = currentButtonState;

    // 6. Despacho Imediato do Payload via Barramento de RF (ESP-NOW)
    esp_now_send(receiverAddress, (uint8_t *) &myData, sizeof(myData));

    delay(15); // Taxa cíclica rigorosa estável em ~66Hz
}
