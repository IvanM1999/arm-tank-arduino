/**
 * @file Teste_Barometro_BMP280.ino
 * @brief Código de teste isolado para o Barómetro BMP280 (I2C) no ESP32
 * @layer Unidade de Laboratório
 */

#include <Wire.h>
#include <Adafruit_BMP280.h>

// Inicializa o objeto do sensor usando a comunicação I2C nativa
Adafruit_BMP280 bmp; 

// Configuração da pressão padrão ao nível do mar na vossa região (em hPa).
// 1013.25 é o padrão global, mas ajustar este valor calibra a precisão da altitude.
#define PRESSAO_NIVEL_DO_MAR_HPA (1013.25)

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("[LAB-BMP280] Inicializando barometro...");

  // Inicializa o sensor no endereço padrão I2C (geralmente 0x76 ou 0x77)
  // Se o sensor não iniciar, tente mudar o parâmetro para 0x76
  if (!bmp.begin(0x76)) {
    Serial.println("❌ [LAB-BMP280] Erro: Sensor nao encontrado! Verifique as conexoes SDA/SCL.");
    while (1); // Trava o código para análise física
  }

  /* Configurações otimizadas para leitura de laboratório */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Modo de operação. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Oversampling de temperatura */
                  Adafruit_BMP280::SAMPLING_X16,    /* Oversampling de pressão */
                  Adafruit_BMP280::FILTER_X16,      /* Filtro contra ruído/vento */
                  Adafruit_BMP280::STANDBY_MS_500); /* Tempo de espera entre leituras */

  Serial.println("✅ [LAB-BMP280] Sensor online e calibrado.");
}

void loop() {
  // Leitura da Temperatura
  float temperatura = bmp.readTemperature();
  
  // Leitura da Pressão Atmosférica (convertida para Pascal)
  float pressao = bmp.readPressure();
  
  // Cálculo aproximado da Altitude com base na pressão local
  float altitude = bmp.readAltitude(PRESSAO_NIVEL_DO_MAR_HPA);

  // Exibe os dados recolhidos no Monitor Serial
  Serial.println("=========================================");
  Serial.print("Temperatura : ");
  Serial.print(temperatura);
  Serial.println(" *C");

  Serial.print("Pressao     : ");
  Serial.print(pressao / 100.0); // Converte Pa para hPa
  Serial.println(" hPa");

  Serial.print("Altitude Est: ");
  Serial.print(altitude);
  Serial.println(" metros");
  Serial.println("=========================================");

  delay(2000); // Aguarda 2 segundos para a próxima amostragem
}
