```cpp
#include <esp_now.h>
#include <WiFi.h>

// Endereço MAC do ESP32 Receptor (Substitua pelo MAC real do seu Receptor)
uint8_t receiverAddress[] = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF};

// Pinos Analógicos do ESP32-C3
const int PIN_X = 0;
const int PIN_Y = 1;

// Coeficiente do Filtro EMA (0.0 a 1.0) - Menor = mais suave, Maior = mais rápido
const float EMA_ALPHA = 0.2; 
float filteredX = 2048.0;
float filteredY = 2048.0;

// Struct otimizada (apenas 2 bytes de payload)
typedef struct struct_message {
    int8_t posX;
    int8_t posY;
} struct_message;

struct_message myData;
esp_now_peer_info_t peerInfo;

// Callback de envio (para depuração e validação de link)
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    // Retorno silencioso para performance; descomente se necessário em lab
    // Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Sucesso" : "Falha");
}

void setup() {
    Serial.begin(115200);
    
    // Configura Wi-Fi em modo Station e desliga sleep mode para menor latência
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);

    if (esp_now_init() != ESP_OK) {
        Serial.println("Erro ao inicializar ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);
    
    // Registra o receptor peer
    memcpy(peerInfo.peer_addr, receiverAddress, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        Serial.println("Falha ao adicionar Peer");
        return;
    }
}

void loop() {
    // Leituras analógicas brutas (0 - 4095)
    int rawX = analogRead(PIN_X);
    int rawY = analogRead(PIN_Y);

    // Filtro de Média Móvel Exponencial (EMA) para eliminar trepidação
    filteredX = (EMA_ALPHA * rawX) + ((1.0 - EMA_ALPHA) * filteredX);
    filteredY = (EMA_ALPHA * rawY) + ((1.0 - EMA_ALPHA) * filteredY);

    // Mapeamento e normalização para int8_t (-100 a 100)
    myData.posX = map((int)filteredX, 0, 4095, -100, 100);
    myData.posY = map((int)filteredY, 0, 4095, -100, 100);

    // Envio contínuo e direto via ESP-NOW
    esp_now_send(receiverAddress, (uint8_t *) &myData, sizeof(myData));

    delay(15); // Taxa de atualização estável (~66Hz) para não saturar o meio
}

```
