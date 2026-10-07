#include <esp_now.h>
#include <WiFi.h>
#include <HardwareSerial.h>

// Inicializa a interface UART2 nos pinos lógicos mapeados para o hardware
HardwareSerial SerialNano(2);

// Parâmetros de Filtros e Cinemática
const int DEADZONE = 10;          // Janela de filtragem contra desvios de centro do joystick
const float SMOOTH_FACTOR = 0.15; // Coeficiente passa-baixas para suavização de transição dos servos

// Variáveis de persistência de estado do Braço Robótico
float currentHeight = 90.0;       // Armazena a cota de elevação atual travada na memória
bool heightLockActive = false;    // Status do recurso de estabilização altitudinal

// Definição idêntica da Struct do Transmissor (Garante alinhamento de memória)
typedef struct struct_message {
    int8_t tankX;
    int8_t tankY;
    int8_t armX;
    int8_t armY;
    int8_t btnElevUp;
    int8_t btnElevDown;
    int8_t btnMode;
} struct_message;

struct_message incomingData;
volatile struct_message pendingData;
volatile bool packetReady = false;
portMUX_TYPE packetMux = portMUX_INITIALIZER_UNLOCKED;

// Filtros de interpolação lógica para rastreamento suave de trajetória
float sBase = 90.0;
float sAlcance = 90.0;
float sElev = 90.0;

// O callback só copia o payload; processamento e UART ficam fora do contexto RF.
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataRaw, int len) {
    if (len != sizeof(struct_message)) return;

    portENTER_CRITICAL(&packetMux);
    memcpy((void *)&pendingData, incomingDataRaw, sizeof(pendingData));
    packetReady = true;
    portEXIT_CRITICAL(&packetMux);
}

void processPendingPacket() {
    bool hasPacket = false;
    portENTER_CRITICAL(&packetMux);
    if (packetReady) {
        memcpy(&incomingData, (const void *)&pendingData, sizeof(incomingData));
        packetReady = false;
        hasPacket = true;
    }
    portEXIT_CRITICAL(&packetMux);
    if (!hasPacket) return;

    int8_t tx = incomingData.tankX;
    int8_t ty = incomingData.tankY;
    int8_t ax = incomingData.armX;
    int8_t ay = incomingData.armY;

    // 1. Aplicação de Filtro de Zona Morta nos Eixos Analógicos
    if (abs(tx) < DEADZONE) tx = 0;
    if (abs(ty) < DEADZONE) ty = 0;
    if (abs(ax) < DEADZONE) ax = 0;
    if (abs(ay) < DEADZONE) ay = 0;

    // 2. Processamento do Controle de Cota por Pressionamento Secundário
    if (incomingData.btnElevUp == 1) currentHeight = min(180.0f, currentHeight + 0.4f);
    if (incomingData.btnElevDown == 1) currentHeight = max(0.0f, currentHeight - 0.4f);

    // 3. Mapeamento de Faixas Dinâmicas para Ângulos Absolutos (0 a 180 Graus)
    float targetServoBase = map(ax, -100, 100, 0, 180);
    float targetServoAlcance = map(ay, -100, 100, 0, 180);
    float targetServoElevacao = currentHeight;

    // 4. Modo de Compensação Cinemática (Trava de Plano Altitudinal)
    heightLockActive = (incomingData.btnMode == 1);
    if (heightLockActive) {
        float alcanceOffset = targetServoAlcance - 90.0;
        const float kCompensacao = 0.38;
        targetServoElevacao = constrain(currentHeight - (alcanceOffset * kCompensacao), 0.0, 180.0);
    }

    // 5. Atenuação de Sobrecarga Mecânica via Interpolação Passa-Baixas (Anti-Jitter)
    sBase += (targetServoBase - sBase) * SMOOTH_FACTOR;
    sAlcance += (targetServoAlcance - sAlcance) * SMOOTH_FACTOR;
    sElev += (targetServoElevacao - sElev) * SMOOTH_FACTOR;

    // 6. Formato enviado para o Nano: <Base,Alcance,Elevacao,VelTanque,DirTanque>
    SerialNano.printf("<%d,%d,%d,%d,%d>\n", (int)sBase, (int)sAlcance,
                      (int)sElev, (int)ty, (int)tx);
}

void setup() {
    Serial.begin(115200);
    
    // Inicializa canal UART2 com o Arduino Nano a 115200 bps (Otimizado para Baixa Latência)
    SerialNano.begin(115200, SERIAL_8N1, 16, 17);

    // Desliga funções de economia de energia do rádio para evitar gargalos de comutação
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Erro ao inicializar ESP-NOW");
        return;
    }

    // Vincula a rotina de interrupção orientada por eventos de rede
    esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
    processPendingPacket();
    delay(1);
}
