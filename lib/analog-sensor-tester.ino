const int pinoAnalogico = A0;

void setup() {
  Serial.begin(9600);
  Serial.println("--- Teste de Leitura Analógica ---");
}

void loop() {
  int valorBruto = analogRead(pinoAnalogico);
  float tensao = valorBruto * (5.0 / 1023.0); // Converte para Volts

  Serial.print("Valor Bruto: ");
  Serial.print(valorBruto);
  Serial.print(" | Tensao: ");
  Serial.print(tensao);
  Serial.println("V");

  delay(500);
}
