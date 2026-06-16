void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // Aguarda a conexão da porta serial (necessário para placas como Leonardo/Micro)
  }
  Serial.println("--- Teste de Comunicação Serial: OK ---");
}

void loop() {
  Serial.println("Placa rodando... Enviando sinal de vida.");
  delay(2000);
}
