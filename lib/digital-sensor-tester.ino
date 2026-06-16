const int pinoDigital = 12; // Altere para o pino que deseja testar

void setup() {
  pinMode(pinoDigital, OUTPUT);
  Serial.begin(9600);
  Serial.println("--- Teste de Saída Digital Ativa ---");
}

void loop() {
  Serial.println("Acionando componente (HIGH)...");
  digitalWrite(pinoDigital, HIGH);
  delay(2000);

  Serial.println("Desligando componente (LOW)...");
  digitalWrite(pinoDigital, LOW);
  delay(2000);
}
