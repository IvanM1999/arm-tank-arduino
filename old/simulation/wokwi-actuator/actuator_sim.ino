#include <Servo.h>

Servo servoBase;
Servo servoAlcance;
Servo servoElevacao;

const byte PIN_MOTOR_ESQ = 5;
const byte PIN_MOTOR_DIR = 6;
const unsigned long LINK_TIMEOUT_MS = 1500;

char frameBuffer[40];
byte frameLength = 0;
bool receivingFrame = false;
unsigned long lastValidFrame = 0;
bool safeMode = true;

int clampCommand(int value) {
  return constrain(value, -100, 100);
}

int commandToPwm(int value) {
  return map(clampCommand(value), -100, 100, 0, 255);
}

void setSafeState() {
  servoBase.write(90);
  servoAlcance.write(90);
  servoElevacao.write(90);
  analogWrite(PIN_MOTOR_ESQ, 0);
  analogWrite(PIN_MOTOR_DIR, 0);
  safeMode = true;
}

void executeFrame(const char *frame) {
  int base;
  int alcance;
  int elevacao;
  int velocidade;
  int direcao;
  int fields = sscanf(frame, "%d,%d,%d,%d,%d", &base, &alcance,
                      &elevacao, &velocidade, &direcao);
  bool valid = fields == 5 && base >= 0 && base <= 180 && alcance >= 0 &&
               alcance <= 180 && elevacao >= 0 && elevacao <= 180 &&
               velocidade >= -100 && velocidade <= 100 && direcao >= -100 &&
               direcao <= 100;
  if (!valid) {
    Serial.println(F("ERR invalid-frame"));
    return;
  }

  servoBase.write(base);
  servoAlcance.write(alcance);
  servoElevacao.write(elevacao);
  analogWrite(PIN_MOTOR_ESQ, commandToPwm(velocidade + direcao));
  analogWrite(PIN_MOTOR_DIR, commandToPwm(velocidade - direcao));
  lastValidFrame = millis();
  safeMode = false;
  Serial.println(F("OK frame"));
}

void readFrames() {
  while (Serial.available()) {
    char incoming = Serial.read();
    if (incoming == '<') {
      frameLength = 0;
      receivingFrame = true;
    } else if (incoming == '>' && receivingFrame) {
      frameBuffer[frameLength] = '\0';
      receivingFrame = false;
      executeFrame(frameBuffer);
    } else if (receivingFrame && frameLength < sizeof(frameBuffer) - 1) {
      frameBuffer[frameLength++] = incoming;
    }
  }
}

void setup() {
  Serial.begin(115200);
  servoBase.attach(9);
  servoAlcance.attach(10);
  servoElevacao.attach(11);
  pinMode(PIN_MOTOR_ESQ, OUTPUT);
  pinMode(PIN_MOTOR_DIR, OUTPUT);
  setSafeState();
  lastValidFrame = millis();
  Serial.println(F("READY actuator-sim"));
}

void loop() {
  readFrames();
  if (!safeMode && millis() - lastValidFrame > LINK_TIMEOUT_MS) {
    setSafeState();
    Serial.println(F("SAFE link-timeout"));
  }
}