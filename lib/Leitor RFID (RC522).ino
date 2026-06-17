#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN); // Inicializa o objeto RFID

void setup() {
  Serial.begin(9600);
  SPI.begin();     // Inicializa o barramento SPI
  rfid.PCD_Init(); // Inicializa o modulo MFRC522
  
  Serial.println("--- Teste de Leitor RFID RC522 ---");
  Serial.println("Aproxime o cartao ou porta-chaves do leitor...");
}

void loop() {
  // Procura por novos cartoes
  if ( ! rfid.PICC_IsNewCardPresent()) {
    return;
  }

  // Seleciona um dos cartoes
  if ( ! rfid.PICC_ReadCardSerial()) {
    return;
  }

  // Exibe o UID (ID unico) no Monitor Serial em formato Hexadecimal
  Serial.print("Tag ID detetada: ");
  String conteudo = "";
  
  for (byte i = 0; i < rfid.uid.size; i++) {
     Serial.print(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(rfid.uid.uidByte[i], HEX);
  }
  Serial.println("\n--------------------------------");

  // Para a leitura atual para evitar leituras repetidas imediatas
  rfid.PICC_HaltA();
}
