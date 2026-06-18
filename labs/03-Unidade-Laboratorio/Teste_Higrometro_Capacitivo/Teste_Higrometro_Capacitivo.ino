/**
 * @file Teste_Higrometro_Capacitivo.ino
 * @brief Código isolado para Higrómetro de Solo Capacitivo v1.2 no ESP32
 * @layer Unidade de Laboratório
 */

// Pino analógico escolhido (GPIO 34 é excelente pois não sofre interferência do Wi-Fi)
const int pinoSensor = 34;

/**
 * ⚠️ NOTA DE CALIBRAÇÃO DE BANCADA:
 * Os sensores analógicos variam de fabrico para fabrico. Precisam de calibrar estes dois valores:
 * 1. Deixem o sensor totalmente SECO no ar e vejam o valor bruto no Monitor Serial. Coloquem esse valor em 'valorSeco'.
 * 2. Mergulhem o sensor num copo de ÁGUA (até à linha limite indicada na placa) e vejam o valor. Coloquem em 'valorMolhado'.
 */
const int valorSeco = 3100;     // Valor lido com o sensor no ar (Exemplo para ADC de 12-bits do ESP32)
const int valorMolhado = 1400;  // Valor lido com o sensor na água

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("[LAB-HIGROMETRO] Inicializando leitura analógica...");
  
  // Configura a resolução do ADC para 12 bits (valores de 0 a 4095)
  analogReadResolution(12);
  
  pinMode(pinoSensor, INPUT);
  Serial.println("✅ [LAB-HIGROMETRO] Configuração concluída. Insira o sensor no substrato.");
}

void loop() {
  // Lê o valor bruto vindo do sensor
  int valorBruto = analogRead(pinoSensor);

  // Mapeia o valor invertido (Como é capacitivo, quanto MAIS água, MENOR é a leitura de tensão)
  // A função map converte a faixa de calibração para uma escala de 0% a 100%
  int percentagemHumidade = map(valorBruto, valorSeco, valorMolhado, 0, 100);

  // Restringe os valores entre 0% e 100% para evitar ruídos fora da calibração
  percentagemHumidade = constrain(percentagemHumidade, 0, 100);

  // Exibe o diagnóstico para ajuste e monitorização
  Serial.print("[Sensor Solo] Bruto: ");
  Serial.print(valorBruto);
  Serial.print(" | Humidade Estimada: ");
  Serial.print(percentagemHumidade);
  Serial.println("%");

  delay(1000); // Amostragem a cada 1 segundo
}
