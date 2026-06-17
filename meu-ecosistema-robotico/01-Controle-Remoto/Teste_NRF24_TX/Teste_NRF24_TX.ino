/**
 * @file Teste_NRF24_TX.ino
 * @brief Código isolado para Transmissão (TX) usando NRF24L01 no ESP32-C3
 * @layer Controle Remoto
 */

#include <SPI.h>
#include <RF24.h>

// Definição dos pinos CE e CSN para o ESP32-C3
#define CE_PIN   7
#define CSN_PIN  3  // Ajustado para liberar espaço no barramento de pinos

// Nota sobre os pinos fixos de Hardware SPI no ESP32-C3:
// MISO -> GPIO 5 | MOSI -> GPIO 6 | SCK -> GPIO 4

RF24 radio(CE_PIN, CSN_PIN);

// Endereço do canal de comunicação (Deve ser o mesmo no receptor)
const byte endereco[6] = "00001";

// Estrutura de dados simples para enviar (Pacote de Teste)
struct PacoteTeste {
  unsigned long contador;
  char mensagem[12];
};

PacoteTeste dadosEnvio;

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("[NRF24-TX] Inicializando modulo transmissor...");

  // Inicializa o chip do rádio via SPI
  if (!radio.begin()) {
    Serial.println("❌ [NRF24-TX] Erro crítico: Chip nao respondeu via SPI!");
    while (1); // Trava o código se houver erro físico de fiação
  }

  // Abre o canal para escrita/transmissão
  radio.openWritingPipe(endereco);
  
  // Configurações para garantir máxima estabilidade em bancada:
  radio.setPALevel(RF24_PA_LOW);  // Potência baixa para testes próximos um do outro
  radio.setDataRate(RF24_1MBPS);   // Velocidade estável de 1 Megabit por segundo
  radio.stopListening();          // Garante que o módulo está focado em TRANSMITIR (TX)

  // Inicializa os dados do pacote
  dadosEnvio.contador = 0;
  strcpy(dadosEnvio.mensagem, "PING_TESTE");

  Serial.println("✅ [NRF24-TX] Inicializacao OK. Transmitindo pacotes...");
}

void loop() {
  dadosEnvio.contador++; // Incrementa o número do pacote enviado

  Serial.print("[NRF24-TX] Enviando pacote nº: ");
  Serial.print(dadosEnvio.contador);

  // Dispara o pacote pelo ar. 
  // O método write retorna true se um receptor enviar um sinal de "Recebido" (ACK)
  bool sucesso = radio.write(&dadosEnvio, sizeof(dadosEnvio));

  if (sucesso) {
    Serial.println(" -> Retorno ACK Recebido com sucesso!");
  } else {
    Serial.println(" -> Enviado (Sem resposta de receptor remoto)");
  }

  delay(1000); // Envia um pacote por segundo
}
