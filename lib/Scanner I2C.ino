#include <Wire.h>

void setup() {
  Wire.begin();
  Serial.begin(9600);
  while (!Serial);
  Serial.println("\n--- Scanner I2C ---");
}

void loop() {
  byte erro, endereco;
  int nDispositivos = 0;

  Serial.println("Escaneando...");

  for (endereco = 1; endereco < 127; endereco++ ) {
    Wire.beginTransmission(endereco);
    erro = Wire.endTransmission();

    if (erro == 0) {
      Serial.print("Dispositivo I2C encontrado no endereco 0x");
      if (endereco < 16) Serial.print("0");
      Serial.print(endereco, HEX);
      Serial.println(" !");
      nDispositivos++;
    }
    else if (erro == 4) {
      Serial.print("Erro desconhecido no endereco 0x");
      if (endereco < 16) Serial.print("0");
      Serial.println(endereco, HEX);
    }
  }
  
  if (nDispositivos == 0) Serial.println("Nenhum dispositivo I2C encontrado\n");
  else Serial.println("Concluido.\n");

  delay(5000); // Aguarda 5 segundos para o próximo escaneamento
}
