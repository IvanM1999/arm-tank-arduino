/**
 * @file Codigo_Unificado_Laboratorio.ino
 * @brief Código unificado de sensores (Barômetro + Higrômetro) para o ESP32
 * @layer Unidade de Laboratório
 */

#include <Wire.h>
#include <Adafruit_BMP280.h>

// --- Configurações do Higrômetro de Solo ---
const int PINO_HIGROMETRO = 34;
// AJUSTE DE CALIBRAÇÃO (Altere conforme os testes de bancada do seu time)
const int VALOR_SECO = 3100;    
const int VALOR_MOLHADO = 1400; 

// --- Instanciamento do Barômetro ---
Adafruit_BMP280 bmp; 
#define PRESSAO_NIVEL_DO_MAR_HPA (1013.25)

// --- Controle de Tempo (Substitutos do delay) ---
unsigned long tempoAnterior = 0;
const long intervaloLeitura = 2000; // Tempo entre amostragens (2 segundos)

// --- Estrutura de Telemetria Interna ---
struct DadosLaboratorio {
  float temperatura;
  float pressao;
  float altitude;
  int umidadeSolo;
};

DadosLaboratorio labData;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("\n=== INICIALIZANDO FIRMWARE UNIFICADO: LABORATÓRIO ===");

  // 1. Configuração do ADC do Higrômetro
  analogReadResolution(12); // Resolução de 12 bits (0-4095)
  pinMode(PINO_HIGROMETRO, INPUT);
  Serial.println("-> Higrômetro Capacitivo: OK");

  // 2. Inicialização do Barômetro I2C (Endereço padrão 0x76)
  if (!bmp.begin(0x76)) {
    Serial.println("❌ ERRO CRÍTICO: BMP280 não encontrado no barramento I2C!");
    while (1); 
  }
  
  // Configurações internas do sensor para reduzir ruído elétrico
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
                  
  Serial.println("-> Barômetro BMP280: OK");
  Serial.println("=== SISTEMA OPERACIONAL E EM EXECUÇÃO ===\n");
}

void loop() {
  unsigned long tempoAtual = millis();

  // Executa a leitura periódica sem congelar o loop principal
  if (tempoAtual - tempoAnterior >= intervaloLeitura) {
    tempoAnterior = tempoAtual;

    lerSensores();
    exibirDiagnostico();
  }

  // O espaço abaixo fica 100% livre para processar o Web Server ou receber comandos futuros
}

/**
 * @brief Realiza a varredura física e o cálculo matemático de todos os sensores
 */
void lerSensores() {
  // Leitura do Barômetro
  labData.temperatura = bmp.readTemperature();
  labData.pressao = bmp.readPressure() / 100.0; // Converte para hPa
  labData.altitude = bmp.readAltitude(PRESSAO_NIVEL_DO_MAR_HPA);

  // Leitura e mapeamento do Higrômetro
  int brutoSolo = analogRead(PINO_HIGROMETRO);
  int percentagem = map(brutoSolo, VALOR_SECO, VALOR_MOLHADO, 0, 100);
  labData.umidadeSolo = constrain(percentagem, 0, 100); // Garante limite entre 0% e 100%
}

/**
 * @brief Formata e envia os dados consolidados para o monitor de depuração
 */
void exibirDiagnostico() {
  Serial.println("┌────────────────────────────────────────┐");
  Serial.println("│        TELEMETRIA DE LABORATÓRIO       │");
  Serial.println("├────────────────────────────────────────┤");
  
  Serial.print("│ Temperatura Ar  : ");
  Serial.print(labData.temperatura, 1);
  Serial.println(" °C                    │");
  
  Serial.print("│ Pressão Atmosf. : ");
  Serial.print(labData.pressao, 1);
  Serial.println(" hPa               │");
  
  Serial.print("│ Altitude Est.   : ");
  Serial.print(labData.altitude, 1);
  Serial.println(" m                 │");
  
  Serial.print("│ Umidade do Solo : ");
  Serial.print(labData.umidadeSolo);
  Serial.println(" %                     │");
  
  Serial.println("└────────────────────────────────────────┘");
}
