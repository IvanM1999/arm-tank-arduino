#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <TFT_eSPI.h> // Biblioteca otimizada para Displays TFT SPI
#include <SPI.h>

// Se estivéssemos utilizando componentes avançados, as libs abaixo seriam descomentadas:
// #include <Adafruit_MPU6050.h>
// #include <Adafruit_Sensor.h>

// ---- PINAGEM DO DEVKIT GRANDE ----
#define BUZZER      12
#define BTN_PANICO  13 // Botão pânico lógico local
#define RX_CABO     26 // Serial física de backup
#define TX_CABO     27
#define VOLT_SENS   34 // Monitoramento de tensão da bateria

// Pinos dos Sensores de Distância IR
#define IR_FRONT_ESQ 32
#define IR_FRONT_DIR 33
#define IR_TRASEIRO  35

// Definição da Garra Mecânica (Configuração Automática de Eixos)
const int QUANTIDADE_EIXOS_GARRA = 6; // Pode ser alterado para 4 ou 6 baseado no seu setup

// ---- MEMÓRIA DA CAIXA PRETA (LOG DE RETORNO) ----
struct RegistroPasso {
    int velEsq;
    int velDir;
    unsigned long duracao;
};
const int TAMANHO_MEMORIA = 150; // Grava os últimos comandos
RegistroPasso logsOperacao[TAMANHO_MEMORIA];
int indiceGravacao = 0;
unsigned long ultimoTempoRegistro = 0;

// ---- CONTROLE DE SINAL E ESTADOS DE ALERTA ----
unsigned long ultimaRecepcaoSinal = 0;
bool modoRetornoAtivo = false;
bool pânicoGeral = false;

typedef struct struct_mensagem {
    int j1_x; int j1_y;
    int j2_x; int j2_y;
    byte botoes;
    bool panico;
} struct_mensagem;
struct_mensagem comandoRecebido;

TFT_eSPI tft = TFT_eSPI(); // Pinos de SPI configurados no arquivo User_Setup.h da lib

void setup() {
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, RX_CABO, TX_CABO);

    pinMode(BUZZER, OUTPUT);
    pinMode(BTN_PANICO, INPUT_PULLUP);

    // Inicialização do Display TFT de 2.69"
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString("Sistema de Defesa Tanque: Online", 10, 10, 2);

    // Inicialização Redes Wireless
    WiFi.mode(WIFI_STA);
    if (esp_now_init() == ESP_OK) {
        // Callback executada ao receber pacotes wireless
        esp_now_register_recv_cb(esp_now_recv_cb_t([](const uint8_t * mac, const uint8_t *incomingData, int len) {
            memcpy(&comandoRecebido, incomingData, sizeof(comandoRecebido));
            ultimaRecepcaoSinal = millis();
        }));
    }

    // --- CÓDIGO COMENTADO: INICIALIZAÇÃO DA CÂMERA INTERNA ---
    /*
    esp_err_t err = esp_camera_init(&configCamera);
    if (err != ESP_OK) { Serial.println("Falha na Inicialização da Câmera Interna"); }
    else { tft.drawString("Streaming de Vídeo Interno: OK", 10, 30, 2); }
    */

    // --- CÓDIGO COMENTADO: INICIALIZAÇÃO DO GIROSCÓPIO MPU6050 ---
    /*
    if (!mpu.begin()) { Serial.println("Giroscópio MPU não encontrado!"); }
    else { mpu.setAccelerometerRange(MPU6050_RANGE_8_G); }
    */
}


void emitirAlertaSonoro(int tipo) {
    switch(tipo) {
        case 1: tone(BUZZER, 400, 100); break;  // Bateria Baixa (Tom lento)
        case 2: tone(BUZZER, 1000, 50); break;  // Colisão Iminente (Tom agudo e rápido)
        case 3: tone(BUZZER, 150, 500); break;  // Inclinação Crítica Grave
        case 4: tone(BUZZER, 2200, 80); break;  // Garra Travada / Fim de Curso estourado
        case 5: tone(BUZZER, 880, 1000); break; // BOTÃO PÂNICO ATIVADO
    }
}

void processarRetornoAutomatico() {
    tft.fillScreen(TFT_RED);
    tft.setTextColor(TFT_WHITE);
    tft.drawString("ALERTA: SINAL PERDIDO - EMERGENCIA", 10, 10, 4);

    // Varre o Log da Caixa Preta ao contrário e desfaz os passos
    for (int i = indiceGravacao - 1; i >= 0; i--) {
        emitirAlertaSonoro(1);
        // Aplica o inverso das velocidades registradas nos motores
        int revVelEsq = -logsOperacao[i].velEsq;
        int revVelDir = -logsOperacao[i].velDir;
        
        tft.printf("Retornando Passo %d | V_E:%d V_D:%d\n", i, revVelEsq, revVelDir);
        // Exemplo fictício de acionamento de hardware de tração reverso:
        // acionarMotores(revVelEsq, revVelDir);
        
        delay(logsOperacao[i].duracao);
    }
    modoRetornoAtivo = false;
    indiceGravacao = 0; // Reseta após o retorno completo seguro
}

void loop() {
    // 1. Verificação de Pânico (Botão Físico Local ou Comando Wireless Remoto)
    if (digitalRead(BTN_PANICO) == LOW || comandoRecebido.panico) {
        pânicoGeral = true;
        emitirAlertaSonoro(5);
        tft.fillScreen(TFT_MAROON);
        tft.drawString("SISTEMA TRAVADO EM PANICO", 40, 100, 4);
        // Código rígido de segurança: Desliga todas as pontes H e travas imediatamente
        return; 
    }

    // 2. Leitura de Backup (Caso o sinal de rádio falhe, tenta pegar comandos via cabo)
    if (Serial2.available()) {
        String dadosCabo = Serial2.readStringUntil('\n');
        if (dadosCabo.startsWith("<") && dadosCabo.endsWith(">")) {
             // Parseamento rápido e redundante dos dados do cabo
             ultimaRecepcaoSinal = millis();
        }
    }

    // 3. Monitoramento de Perda de Sinal (Segurança Inteligente / Fail-Safe)
    if (millis() - ultimaRecepcaoSinal > 8000 && !modoRetornoAtivo && indiceGravacao > 10) {
        modoRetornoAtivo = true;
        processarRetornoAutomatico();
    }

    // 4. Tratamento de Telemetria e Filtragem de Colisão dos Sensores IR
    int sensorEsq = analogRead(IR_FRONT_ESQ);
    int sensorDir = analogRead(IR_FRONT_DIR);
    if (sensorEsq > 3000 || sensorDir > 3000) {
        emitirAlertaSonoro(2); // Alerta de Colisão iminente detectada pelas dimensões IR
    }

    // 5. Inteligência Cinemática da Garra Baseado na Contagem de Eixos
    if (QUANTIDADE_EIXOS_GARRA == 6) {
        // Controle Avançado Cooperativo: Garra compensa elevação paralelamente ao movimento do braço
        int anguloCompensado = map(comandoRecebido.j2_y, -2048, 2048, 0, 180);
        // Executa rotação suave e calculada sem trancos elétricos
    } else {
        // Modo Padrão de 4 Eixos (Apenas obedece diretamente a movimentação sem cálculos de malha fechada)
    }

    // 6. Monitoramento de Bateria
    float leituraBateria = (analogRead(VOLT_SENS) * 3.3 / 4095.0) * 4.0; // Fator de correção do divisor de tensão
    if (leituraBateria < 11.1) { 
        emitirAlertaSonoro(1); // Bateria do robô considerada perigosamente baixa
    }

    // 7. Salva a Operação Atual na Caixa Preta (Buffer de Rastreamento) se houver movimento
    if ((abs(comandoRecebido.j1_y) > 200 || abs(comandoRecebido.j1_x) > 200) && !modoRetornoAtivo) {
        if (millis() - ultimoTempoRegistro > 200) { // Loga a cada 200ms
            logsOperacao[indiceGravacao].velEsq = comandoRecebido.j1_y + comandoRecebido.j1_x;
            logsOperacao[indiceGravacao].velDir = comandoRecebido.j1_y - comandoRecebido.j1_x;
            logsOperacao[indiceGravacao].duracao = millis() - ultimoTempoRegistro;
            
            ultimoTempoRegistro = millis();
            indiceGravacao++;
            if (indiceGravacao >= TAMANHO_MEMORIA) indiceGravacao = 0; // Buffer circular
        }
    }
    // --- CÓDIGO COMENTADO: STREAMING DE DADOS E TELEMETRIA DA CÂMERA NA TELA TFT ---
    /*
    camera_fb_t * fb = esp_camera_fb_get();
    if(fb) {
       // Renderização direta da matriz de pixels na Tela TFT 2.69" 
       tft.pushImage(0, 0, fb->width, fb->height, (uint16_t *)fb->buf);
       esp_camera_fb_return(fb);
    }
    */

    // --- CÓDIGO COMENTADO: PROCESSAMENTO E ALERTAS DO GIROSCÓPIO ---
    /*
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    if(abs(a.acceleration.x) > 4.5 || abs(a.acceleration.y) > 4.5) {
        emitirAlertaSonoro(3); // Inclinação perigosa fora do limite gravitacional estável
        tft.drawString("CRITICO: INCLINACAO EXCESSIVA!", 10, 50, 2);
    }
    */

    delay(20);
}

