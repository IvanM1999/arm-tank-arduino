/**
 * @file Teste_Motores_PonteH.ino
 * @brief Código isolado para teste de motores DC via Ponte H no Arduino Nano
 * @layer Unidade de Tração (Tanque)
 */

// Definição dos pinos de controle do Motor Esquerdo
const int pinIN1 = 3;  // Controle de direção (PWM opcional)
const int pinIN2 = 4;  // Controle de direção

// Definição dos pinos de controle do Motor Direito
const int pinIN3 = 5;  // Controle de direção (PWM opcional)
const int pinIN4 = 6;  // Controle de direção

// Definição dos pinos de ativação/velocidade (Se a sua Ponte H usar jumpers ENA/ENB)
// Se a sua placa não usar pinos Enable separados, ignore-os e controle a velocidade direto nos pinos IN usando analogWrite.
const int pinENA = 11; // Pino PWM para velocidade do Motor Esquerdo
const int pinENB = 10; // Pino PWM para velocidade do Motor Direito

void setup() {
  Serial.begin(9600);
  Serial.println("[MOTORES] Configurando pinos de saida...");

  // Configura todos os pinos de controle do motor como saídas
  pinMode(pinIN1, OUTPUT);
  pinMode(pinIN2, OUTPUT);
  pinMode(pinIN3, OUTPUT);
  pinMode(pinIN4, OUTPUT);
  pinMode(pinENA, OUTPUT);
  pinMode(pinENB, OUTPUT);

  Serial.println("✅ [MOTORES] Pronto. Iniciando sequencia de testes...");
}

void loop() {
  // --- TESTE 1: Mover para a Frente com Velocidade Máxima (255) ---
  Serial.println("[MOTORES] Movimento: FRENTE (Velocidade Maxima)");
  analogWrite(pinENA, 255);
  analogWrite(pinENB, 255);
  digitalWrite(pinIN1, HIGH);
  digitalWrite(pinIN2, LOW);
  digitalWrite(pinIN3, HIGH);
  digitalWrite(pinIN4, LOW);
  delay(3000); // Mantém por 3 segundos

  // --- TESTE 2: Travar os motores (Paragem de emergência) ---
  Serial.println("[MOTORES] Movimento: PARADO");
  digitalWrite(pinIN1, LOW);
  digitalWrite(pinIN2, LOW);
  digitalWrite(pinIN3, LOW);
  digitalWrite(pinIN4, LOW);
  delay(1000); // Fica parado por 1 segundo

  // --- TESTE 3: Mover para Trás com Meia Velocidade (150) ---
  Serial.println("[MOTORES] Movimento: TRAS (Meia Velocidade)");
  analogWrite(pinENA, 150);
  analogWrite(pinENB, 150);
  digitalWrite(pinIN1, LOW);
  digitalWrite(pinIN2, HIGH);
  digitalWrite(pinIN3, LOW);
  digitalWrite(pinIN4, HIGH);
  delay(3000);

  // --- TESTE 4: Girar no Próprio Eixo (Curva para a Direita) ---
  Serial.println("[MOTORES] Movimento: CURVA DIREITA");
  analogWrite(pinENA, 180);
  analogWrite(pinENB, 180);
  digitalWrite(pinIN1, HIGH); // Motor Esquerdo para a frente
  digitalWrite(pinIN2, LOW);
  digitalWrite(pinIN3, LOW);  // Motor Direito para trás
  digitalWrite(pinIN4, HIGH);
  delay(2000);

  // Desliga tudo antes de reiniciar o loop
  digitalWrite(pinIN1, LOW);
  digitalWrite(pinIN2, LOW);
  digitalWrite(pinIN3, LOW);
  digitalWrite(pinIN4, LOW);
  Serial.println("[MOTORES] Fim do ciclo de testes. Reiniciando em 2 segundos...");
  delay(2000);
}
