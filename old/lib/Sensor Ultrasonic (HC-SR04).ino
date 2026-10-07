const int pinoTrig = 8;
const int pinoEcho = 7;

void setup() {
  pinMode(pinoTrig, OUTPUT); 
  pinMode(pinoEcho, INPUT);  
  Serial.begin(9600);
  Serial.println("--- Teste de Sensor Ultrassonico HC-SR04 ---");
}

void loop() {
  // Limpa o pino Trig
  digitalWrite(pinoTrig, LOW);
  delayMicroseconds(2);
  
  // Envia um pulso de 10 microsegundos
  digitalWrite(pinoTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinoTrig, LOW);
  
  // Le o tempo de viagem da onda (em microsegundos)
  long duracao = pulseIn(pinoEcho, HIGH);
  
  // Calcula a distancia em centimetros
  float distanciaCM = duracao * 0.0343 / 2;
  
  // Exibe no Monitor Serial
  if (distanciaCM >= 400 || distanciaCM <= 2) {
    Serial.println("Fora de alcance (2cm - 400cm)");
  } else {
    Serial.print("Distancia: ");
    Serial.print(distanciaCM);
    Serial.println(" cm");
  }
  
  delay(500); // Aguarda meio segundo para a proxima leitura
}
