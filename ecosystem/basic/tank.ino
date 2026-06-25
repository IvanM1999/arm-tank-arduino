```cpp
#include <Servo.h>

Servo servoBase;
Servo servoGarra;

// Constantes de Controle de Fluxo Serial
const byte numChars = 32;
char receivedChars[numChars];
boolean newData = false;

// Controle de Fail-Safe
unsigned long lastPacketTime = 0;
const unsigned long TIMEOUT_THRESHOLD = 2000; // 2 segundos de timeout
bool isSafeMode = false;

void setup() {
    // Inicializa a Serial no mesmo Baud Rate do ESP32
    Serial.begin(115200);
    
    servoBase.attach(9);
    servoGarra.attach(10);
    
    // Posição inicial segura
    servoBase.write(90);
    servoGarra.write(90);
    
    lastPacketTime = millis();
}

void loop() {
    recvWithStartEndMarkers();
    parseAndExecute();
    checkFailSafe();
}

// Máquina de estados robusta para leitura de pacotes delimitados sem bloquear o fluxo
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
                    ndx = numChars - 1; // Previne estouro de buffer
                }
            }
            else {
                receivedChars[ndx] = '\0'; // Termina a string
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

// Processa a string de forma segura e atua nos servos
void parseAndExecute() {
    if (newData == true) {
        // Atualiza o watchdog do Fail-Safe imediatamente após receber pacote íntegro
        lastPacketTime = millis();
        isSafeMode = false;

        char * strtokIndx;

        // Extrai o primeiro ângulo (Servo 1)
        strtokIndx = strtok(receivedChars,",");
        if (strtokIndx != NULL) {
            int valServo1 = atoi(strtokIndx);
            
            // Validação de integridade do dado (0 a 180) antes de escrever no hardware
            if (valServo1 >= 0 && valServo1 <= 180) {
                servoBase.write(valServo1);
            }
        }
        
        // Extrai o segundo ângulo (Servo 2)
        strtokIndx = strtok(NULL, ",");
        if (strtokIndx != NULL) {
            int valServo2 = atoi(strtokIndx);
            
            if (valServo2 >= 0 && valServo2 <= 180) {
                servoGarra.write(valServo2);
            }
        }
        
        newData = false;
    }
}

// Rotina de Fail-Safe não bloqueante
void checkFailSafe() {
    if ((millis() - lastPacketTime > TIMEOUT_THRESHOLD) && !isSafeMode) {
        servoBase.write(90);
        servoGarra.write(90);
        isSafeMode = true;
        // Imprime em lab para debug pela USB (se não conflitar com a linha)
        // Serial.println("ALERTA: Fail-safe Ativado!"); 
    }
}

```
