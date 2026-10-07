/**
 * @file Teste_NRF24_RX.ino
 * @brief Código isolado para Recepção (RX) usando NRF24L01 no Arduino Nano
 * @layer Unidade de Tração (Tanque)
 */

#include <SPI.h>
#include <RF24.h>

// Definição dos pinos CE e CSN padrão para o Arduino Nano
#define CE_PIN   9
#define CSN_PIN 10

// Pinos fixos de Hardware SPI no Arduino Nano:
// MOSI -> D11 | MISO -> D12 | SCK -> D13

RF24 radio(CE_PIN, CSN_PIN);

// Endereço do canal de comunicação (Deve ser IGUAL ao do Transmissor)
const byte endereco[6] = "00001";

// Estrutura de dados idêntica à do Transmissor para desempacotar corretamente
struct PacoteTeste {
  unsigned long contador;
  char mensagem[12];
};

PacoteTeste dadosRecebidos;

void setup() {
  Serial.begin(9600);
  while (!Serial); // Aguarda abertura do Monitor Serial
  Serial.println("[NRF24-RX] Inicializando modulo recetor...");

  // Inicializa o chip do rádio via SPI
  if (!radio.begin()) {
    Serial.println("❌ [NRF24-RX] Erro critico: Chip nao respondeu via SPI!");
    while (1); // Trava o código para análise física de fiação
  }

  // Abre o canal configurado para leitura (Escuta)
  radio.openReadingPipe(1, endereco);
  
  // Configurações idênticas às do TX para haver sincronia:
  radio.setPALevel(RF24_PA_LOW);  // Potência baixa para testes em bancada
  radio.setDataRate(RF24_1MBPS);   // Velocidade de 1 Megabit por segundo
  
  // Ativa o modo de escuta ativa
  radio.startListening();          

  Serial.println("✅ [NRF24-RX] Inicializacao OK. Aguardando pacotes no ar...");
}

void loop() {
  // Verifica se há dados pendentes no buffer do rádio
  if (radio.available()) {
    
    // Lê os dados e armazena na nossa estrutura
    radio.read(&dadosRecebidos, sizeof(dadosRecebidos));

    // Exibe os dados recebidos no Monitor Serial
    Serial.print("[NRF24-RX] Pacote Recebido com Sucesso! -> ");
    Serial.print("ID: ");
    Serial.print(dadosRecebidos.contador);
    Serial.print(" | Msg: ");
    Serial.println(dadosRecebidos.mensagem);
  }
}
