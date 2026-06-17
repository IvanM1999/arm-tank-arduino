const int pinoPIR = 2;

void setup() {
  pinMode(pinoPIR, INPUT);
  Serial.begin(9600);
  Serial.println("--- Teste de Sensor de Presenca PIR ---");
  Serial.println("Aguardando 10s para estabilizacao do sensor...");
  delay(10000); // O sensor PIR precisa de um tempo para se calibrar ao ambiente
  Serial.println("Sensor Pronto!");
}

void loop() {
  int estadoPIR = digitalRead(pinoPIR);
  
  if (estadoPIR == HIGH) {
    Serial.println("[ ALERTA ] Movimento detetado!");
  } else {
    Serial.println("Ambiente calmo... Sem movimento.");
  }
  
  delay(1000);
}
