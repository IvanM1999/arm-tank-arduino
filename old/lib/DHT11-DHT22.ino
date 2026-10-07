#include <DHT.h>

#define DHTPIN 2          // Pino digital conectado ao sensor
#define DHTTYPE DHT11     // Mude para DHT22 se estiver usando o modelo branco

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(9600);
  Serial.println("--- Teste de Sensor DHT ---");
  dht.begin();
}

void loop() {
  delay(2000); // O DHT precisa de pelo menos 2 segundos entre as leituras

  float umidade = dht.readHumidity();
  float temperatura = dht.readTemperature(); // Em Celsius

  // Verifica se a leitura falhou
  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("Falha ao ler o sensor DHT! Verifique a fiação.");
    return;
  }

  Serial.print("Umidade: ");
  Serial.print(umidade);
  Serial.print("% | Temperatura: ");
  Serial.print(temperatura);
  Serial.println("°C");
}
