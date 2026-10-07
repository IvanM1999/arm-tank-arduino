#include <Servo.h>

// Instanciação dos objetos de controle dos atuadores (Servos SG90)
Servo servoBase;
Servo servoAlcance;
Servo servoElevacao;

// Pinos de Atuação da Unidade de Tração (Ponte H / Chassi do Tanque)
const int PIN_MOTOR_ESQ = 5;
const int PIN_MOTOR_DIR = 6;

// Constantes de Gerenciamento e Controle do Fluxo Serial UART
const byte numChars = 40; // Buffer estendido para suportar a nova string expandida
char receivedChars[numChars];
bool newData = false;

// Parâmetros de Segurança Crítica (Watchdog por Software / Fail-Safe)
unsigned long lastPacketTime = 0;
const unsigned long TIMEOUT_THRESHOLD = 2000; // Tolerância máxima de 2 segundos de queda de link
bool isSafeMode = false;

void setup() {
    // Inicialização da interface serial idêntica ao baud rate do Cérebro ESP32
    Serial.begin(115200);
    
    // Vinculação dos canais físicos de PWM aos Servos
    servoBase.attach(9);
    servoAlcance.attach(10);
    servoElevacao.attach(11);
    
    // Configuração dos pinos da Ponte H do chassi
    pinMode(PIN_MOTOR_ESQ, OUTPUT);
    pinMode(PIN_MOTOR_DIR, OUTPUT);
    
    // Inicialização segura em modo estacionário / repouso (90 Graus)
    executarPosicaoSegura();
    
    lastPacketTime = millis();
}

void loop() {
    recvWithStartEndMarkers(); // Máquina de estados de varredura do buffer UART
    parseAndExecute();         // Decodificador de tokens e atuação direta no hardware
    checkFailSafe();           // Monitor de integridade temporal do link
}

// Máquina de estados não-bloqueante para isolar o frame de dados delimitado por '<' e '>'
void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
 
    while (Serial.available() > 0 && newData == false) {
        rc = Serial.read();

        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1; // Salvaguarda contra estouro de memória (Buffer Overflow)
                }
            }
            else {
                receivedChars[ndx] = '\0'; // Finalizador de string nula (C-String)
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }
        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
}

// Só um frame completo e dentro das faixas permitidas pode atualizar o hardware.
void parseAndExecute() {
    if (!newData) return;

    int valBase;
    int valAlcance;
    int valElev;
    int velChassi;
    int dirChassi;
    int fields = sscanf(receivedChars, "%d,%d,%d,%d,%d", &valBase, &valAlcance,
                        &valElev, &velChassi, &dirChassi);
    bool validFrame = fields == 5 && valBase >= 0 && valBase <= 180 &&
                      valAlcance >= 0 && valAlcance <= 180 && valElev >= 0 &&
                      valElev <= 180 && velChassi >= -100 && velChassi <= 100 &&
                      dirChassi >= -100 && dirChassi <= 100;

    if (validFrame) {
        servoBase.write(valBase);
        servoAlcance.write(valAlcance);
        servoElevacao.write(valElev);

        int leftCommand = constrain(velChassi + dirChassi, -100, 100);
        int rightCommand = constrain(velChassi - dirChassi, -100, 100);
        analogWrite(PIN_MOTOR_ESQ, map(leftCommand, -100, 100, 0, 255));
        analogWrite(PIN_MOTOR_DIR, map(rightCommand, -100, 100, 0, 255));

        lastPacketTime = millis();
        isSafeMode = false;
    }

    newData = false;
}

// Rotina preventiva de Fail-Safe não-bloqueante
void checkFailSafe() {
    if ((millis() - lastPacketTime > TIMEOUT_THRESHOLD) && !isSafeMode) {
        executarPosicaoSegura();
        isSafeMode = true;
    }
}

// Força o recolhimento e parada mecânica imediata do ecossistema robótico
void executarPosicaoSegura() {
    servoBase.write(90);
    servoAlcance.write(90);
    servoElevacao.write(90);
    
    // Corta totalmente a tensão aplicada nos motores DC de tração
    analogWrite(PIN_MOTOR_ESQ, 0);
    analogWrite(PIN_MOTOR_DIR, 0);
}
