```cpp
#include <esp_now.h>
#include <WiFi.h>
#include <HardwareSerial.h>

// Inicializa a UART2 nos pinos padrão (TX2=17, RX2=16)
HardwareSerial SerialNano(2);

const int DEADZONE = 10;
float smoothX = 90.0;
float smoothY = 90.0;
const float SMOOTH_FACTOR = 0.15; // Filtro passa-baixas para suavização dos servos

typedef struct struct_message {
    int8_t posX;
    int8_t posY;
} struct_message;

struct_message incomingData;

// Callback executado ao receber pacote do Transmissor
void OnDataRecv(const esp_now_recv_info *info, const uint8_t *incomingDataRaw, int len) {
    if (len == sizeof(struct_message)) {
        memcpy(&incomingData, incomingDataRaw, sizeof(incomingData));
        
        int8_t x = incomingData.posX;
        int8_t y = incomingData.posY;

        // Aplicação da Zona Morta (Deadzone)
        if (abs(x) < DEADZONE) x = 0;
        if (abs(y) < DEADZONE) y = 0;

        // Converte a faixa (-100 a 100) para ângulos absolutos (0 a 180)
        int targetAngleX = map(x, -100, 100, 0, 180);
        int targetAngleY = map(y, -100, 100, 0, 180);

        // Suavização cinemática (Interpolação Linear / Filtro Passa-Baixas)
        smoothX = smoothX + (targetAngleX - smoothX) * SMOOTH_FACTOR;
        smoothY = smoothY + (targetAngleY - smoothY) * SMOOTH_FACTOR;

        // Envio do pacote delimitado robusto via UART (<Angulo1,Angulo2>)
        SerialNano.print('<');
        SerialNano.print((int)smoothX);
        SerialNano.print(',');
        SerialNano.print((int)smoothY);
        SerialNano.println('>');
    }
}

void setup() {
    Serial.begin(115200);
    
    // Inicializa comunicação com o Nano a 115200 bps (Alta velocidade/baixa latência)
    SerialNano.begin(115200, SERIAL_8N1, 16, 17);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Erro ao inicializar ESP-NOW");
        return;
    }

    esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
    // O cérebro opera baseado em interrupções orientadas por eventos (Callback). 
    // O loop fica livre para tarefas secundárias ou sleep leve.
    delay(100);
}

```
