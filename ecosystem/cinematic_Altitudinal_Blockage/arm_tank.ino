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
boolean newData = false;

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

// Tokenização segura e conversão numérica imediata do payload
void parseAndExecute() {
    if (newData == true) {
        // Atualização imediata do Watchdog temporal do sistema
        lastPacketTime = millis();
        isSafeMode = false;

        char * strtokIndx;

        // Token 1: Ângulo do Servo da Base (Rotação)
        strtokIndx = strtok(receivedChars, ",");
        if (strtokIndx != NULL) {
            int valBase = atoi(strtokIndx);
            if (valBase >= 0 && valBase <= 180) servoBase.write(valBase);
        }
        
        // Token 2: Ângulo do Servo de Alcance (Distância)
        strtokIndx = strtok(NULL, ",");
        if (strtokIndx != NULL) {
            int valAlcance = atoi(strtokIndx);
            if (valAlcance >= 0 && valAlcance <= 180) servoAlcance.write(valAlcance);
        }

        // Token 3: Ângulo do Servo de Elevação (Calculado/Compensado pelo Cérebro)
        strtokIndx = strtok(NULL, ",");
        if (strtokIndx != NULL) {
            int valElev = atoi(strtokIndx);
            if (valElev >= 0 && valElev <= 180) servoElevacao.write(valElev);
        }

        // Token 4: Velocidade Linear do Chassi (Eixo Y do Joystick do Tanque)
        strtokIndx = strtok(NULL, ",");
        int velChassi = 0;
        if (strtokIndx != NULL) {
            velChassi = atoi(strtokIndx); // Faixa recebida: -100 a 100
        }

        // Token 5: Direção/Curva do Chassi (Eixo X do Joystick do Tanque)
        strtokIndx = strtok(NULL, ",");
        int dirChassi = 0;
        if (strtokIndx != NULL) {
            dirChassi = atoi(strtokIndx); // Faixa recebida: -100 a 100
        }

        // Misturador Cinemático de Tração Diferencial Simples (Ponte H)
        // Converte eixos mistos em potências individuais para os motores esquerdo e direito
        int motorEsqPwr = map(velChassi + dirChassi, -100, 100, 0, 255);
        int motorDirPwr = map(velChassi - dirChassi, -100, 100, 0, 255);

        // Restringe os valores aos limites PWM de hardware (8-bits)
        motorEsqPwr = constrain(motorEsqPwr, 0, 255);
        motorDirPwr = constrain(motorDirPwr, 0, 255);

        // Escrita direta nos drivers da Ponte H
        analogWrite(PIN_MOTOR_ESQ, motorEsqPwr);
        analogWrite(PIN_MOTOR_DIR, motorDirPwr);
        
        newData = false; // Libera a máquina de estados serial para o próximo ciclo
    }
}

// Rotina preventiva de Fail-Safe não-bloqueante
void checkFailSafe() {
    if ((millis() - lastPacketTime > TIMEOUT_THRESHOLD) && !isSafeMode) {
        executarPosicaoSegura();
        isSafeMode = true;
    }
}

// Força o recolhimento e parada mecânica imediata do ecossistema robótico
void ejecutarPosicaoSegura() {
    servoBase.write(90);
    servoAlcance.write(90);
    servoElevacao.write(90);
    
    // Corta totalmente a tensão aplicada nos motores DC de tração
    analogWrite(PIN_MOTOR_ESQ, 0);
    analogWrite(PIN_MOTOR_DIR, 0);
}
