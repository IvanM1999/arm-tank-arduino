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

// Filtros de interpolação lógica para rastreamento suave de trajetória
float sBase = 90.0;
float sAlcance = 90.0;
float sElev = 90.0;

// Callback assíncrono disparado imediatamente após o recebimento do pacote RF
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataRaw, int len) {
    if (len == sizeof(struct_message)) {
        // Extração direta do buffer de rede para a struct estática
        memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));

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
        // Ajusta a referência da altura mantendo a última posição estável ao soltar o switch
        if (incomingData.btnElevUp == 1) {
            currentHeight += 0.4; // Incremento controlado por clock
            if (currentHeight > 180.0) currentHeight = 180.0;
        } else if (incomingData.btnElevDown == 1) {
            currentHeight -= 0.4; // Decremento controlado por clock
            if (currentHeight < 0.0) currentHeight = 0.0;
        }

        // 3. Mapeamento de Faixas Dinâmicas para Ângulos Absolutos (0 a 180 Graus)
        float targetServoBase     = map(ax, -100, 100, 0, 180);
        float targetServoAlcance  = map(ay, -100, 100, 0, 180);
        float targetServoElevacao = currentHeight;

        // 4. Modo de Compensação Cinemática (Trava de Plano Altitudinal)
        heightLockActive = (incomingData.btnMode == 1);
        
        if (heightLockActive) {
            /* 
               Algoritmo de Acoplamento Geométrico:
               Quando a garra avança (Alcance se afasta de 90), a gravidade e o braço mecânico 
               deslocam a garra para baixo. O fator de acoplamento (kCompensacao) corrige isso 
               erguendo o servo de elevação proporcionalmente para estabilizar a garra no mesmo plano horizontal.
            */
            float alcanceOffset = targetServoAlcance - 90.0;
            const float kCompensacao = 0.38; // Ganho calibrado para garras estruturais multipartes
            
            targetServoElevacao = currentHeight - (alcanceOffset * kCompensacao);
            targetServoElevacao = constrain(targetServoElevacao, 0.0, 180.0);
        }

        // 5. Atenuação de Sobrecarga Mecânica via Interpolação Passa-Baixas (Anti-Jitter)
        sBase    = sBase + (targetServoBase - sBase) * SMOOTH_FACTOR;
        sAlcance = sAlcance + (targetServoAlcance - sAlcance) * SMOOTH_FACTOR;
        sElev    = sElev + (targetServoElevacao - sElev) * SMOOTH_FACTOR;

        // 6. Serialização e Despacho Serial em Frame Delimitado Robusto
        // Formato enviado para o Nano: <Base,Alcance,Elevacao,VelTanque,DirTanque>
        SerialNano.print('<');
        SerialNano.print((int)sBase);          SerialNano.print(',');
        SerialNano.print((int)sAlcance);       SerialNano.print(',');
        SerialNano.print((int)sElev);          SerialNano.print(',');
        SerialNano.print((int)ty);             SerialNano.print(',');
        SerialNano.print((int)tx);
        SerialNano.println('>');
    }
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
    // Código orientado a interrupção. O loop permanece em sleep consciente 
    // liberando o core do processador para priorizar os callbacks de recepção RF.
    delay(100);
}
