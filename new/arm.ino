#include <Servo.h>
#include <EEPROM.h>
#include <math.h>

// ============================================================================
// 1. CONFIGURAÇÕES DE HARDWARE E ESPECIFICAÇÕES DO SERVO
// ============================================================================
// Altere estes valores conforme o modelo físico do seu projeto:
const char* SERVO_MODELO = "SG90_Micro";
const int SERVO_ANG_MIN = 0;
const int SERVO_ANG_MAX = 180;
const int SERVO_FORCA_GRAMAS = 1800; // Ex: 1.8kg.cm (1800g) para SG90 | 500g para micro servos menores

// Dimensões físicas dos braços em milímetros (mm) para cálculo do Domo e Altura
const float L1_BASE_ALTURA = 50.0;  // Altura do chão até o eixo do Ombro
const float L2_BRACO = 100.0;       // Comprimento do segmento Ombro -> Cotovelo
const float L3_ANTEBRACO = 100.0;   // Comprimento do segmento Cotovelo -> Ponta da Garra

// Pinos dos Servos (D2 a D9)
const int PINO_SERVOS[8] = {2, 3, 4, 5, 6, 7, 8, 9};

// Pinos de Detecção Automática de Configuração (Ligar ao GND para ativar)
const int PINO_SENSE_6 = 11;
const int PINO_SENSE_8 = 12;

// Pinos dos Joysticks e Botões
const int JOY1_X = A0;
const int JOY1_Y = A1;
const int JOY2_X = A2;
const int JOY2_Y = A3;
const int BTN_SAVE = A4;
const int BTN_PLAY = A5;
const int BTN_LAYER = 10; // Alterna controle entre Servos 1-4 e Servos 5-8

// ============================================================================
// 2. VARIÁVEIS GLOBAIS E ESTRUTURAS
// ============================================================================
Servo bracoServos[8];
int qtdServosAtivos = 4;
float posAtual[8] = {90, 90, 90, 90, 90, 90, 90, 90}; // Inicia todos em 90 graus
bool camadaSuperior = false; // false = controla S1..S4 | true = controla S5..S8

// Memória EEPROM (Armazena até 15 passos de 8 servos)
const int MAX_PASSOS = 15;
int totalPassosSalvos = 0;

// Temporizadores sem bloqueio (millis)
unsigned long ultimoTempoSerial = 0;
unsigned long ultimoTempoLoop = 0;

// ============================================================================
// 3. SETUP E INICIALIZAÇÃO SEGURA (REBOOT PARA 90 GRAUS)
// ============================================================================
void setup() {
  Serial.begin(115200); // Velocidade alta para telemetria fluida
  
  // Configura pinos de detecção e botões com resistor Pull-Up interno
  pinMode(PINO_SENSE_6, INPUT_PULLUP);
  pinMode(PINO_SENSE_8, INPUT_PULLUP);
  pinMode(BTN_SAVE, INPUT_PULLUP);
  pinMode(BTN_PLAY, INPUT_PULLUP);
  pinMode(BTN_LAYER, INPUT_PULLUP);

  // Detecta quantos servos estão configurados no sistema
  detectarSistemaServos();

  // Recupera quantidade de passos salvos na EEPROM (Endereço 0)
  totalPassosSalvos = EEPROM.read(0);
  if (totalPassosSalvos > MAX_PASSOS || totalPassosSalvos < 0) {
    totalPassosSalvos = 0; // Limpa lixo de memória na primeira gravação
  }

  // Soft-Start: Inicializa cada servo ativo diretamente na posição neutra (90°)
  Serial.println(F("INICIALIZANDO SISTEMA ROBOTICO..."));
  for (int i = 0; i < qtdServosAtivos; i++) {
    bracoServos[i].attach(PINO_SERVOS[i]);
    bracoServos[i].write(90);
    posAtual[i] = 90.0;
    delay(150); // Evita pico de corrente simultâneo no reboot
  }

  imprimirCabecalhoSistema();
}

// ============================================================================
// 4. LOOP PRINCIPAL
// ============================================================================
void loop() {
  unsigned long tempoAtual = millis();

  // Atualiza leitura dos controles a cada 20ms (50Hz - padrão servo analógico)
  if (tempoAtual - ultimoTempoLoop >= 20) {
    ultimoTempoLoop = tempoAtual;
    
    verificarBotoes();
    lerJoysticksEMover();
  }

  // Envia telemetria completa na Serial a cada 250ms
  if (tempoAtual - ultimoTempoSerial >= 250) {
    ultimoTempoSerial = tempoAtual;
    enviarTelemetriaSerial();
  }
}

// ============================================================================
// 5. FUNÇÕES DE DETECÇÃO E CINEMÁTICA (DOMO E ALTURA)
// ============================================================================
void detectarSistemaServos() {
  // Se o pino D12 estiver ligado ao GND, ativa 8 servos
  if (digitalRead(PINO_SENSE_8) == LOW) {
    qtdServosAtivos = 8;
  } 
  // Se o pino D11 estiver ligado ao GND, ativa 6 servos
  else if (digitalRead(PINO_SENSE_6) == LOW) {
    qtdServosAtivos = 6;
  } 
  // Caso contrário, opera no modo padrão de 4 servos
  else {
    qtdServosAtivos = 4;
  }
}

// Calcula a altura Z (em mm) e o raio atual no domo de operação
void calcularCinematica(float &alturaZ, float &raioAtual, float &raioMaximo) {
  raioMaximo = L2_BRACO + L3_ANTEBRACO; // Alcance máximo horizontal (Domo)

  // Converte ângulos dos servos Ombro (S2) e Cotovelo (S3) para radianos
  // Considera 90 graus como posição vertical/neutra de referência
  float radOmbro = radians(posAtual[1]); 
  float radCotovelo = radians(posAtual[2] - 90.0);

  // Projeção trigonométrica 2D no plano vertical
  float zOmbro = L2_BRACO * sin(radOmbro);
  float zCotovelo = L3_ANTEBRACO * sin(radOmbro + radCotovelo);
  
  alturaZ = L1_BASE_ALTURA + zOmbro + zCotovelo;
  if (alturaZ < 0) alturaZ = 0; // Limite físico da mesa/chão

  float rOmbro = L2_BRACO * cos(radOmbro);
  float rCotovelo = L3_ANTEBRACO * cos(radOmbro + radCotovelo);
  raioAtual = abs(rOmbro + rCotovelo);
}

// ============================================================================
// 6. LEITURA DE JOYSTICKS COM ZONA MORTA (DEADZONE)
// ============================================================================
void lerJoysticksEMover() {
  int leituras[4] = {
    analogRead(JOY1_X),
    analogRead(JOY1_Y),
    analogRead(JOY2_X),
    analogRead(JOY2_Y)
  };

  // Define quais 4 servos os joysticks estão controlando no momento
  int offsetServo = (camadaSuperior && qtdServosAtivos > 4) ? 4 : 0;

  for (int i = 0; i < 4; i++) {
    int indiceServo = i + offsetServo;
    if (indiceServo >= qtdServosAtivos) break;

    int erro = leituras[i] - 512; // Centro do potenciômetro é ~512

    // Zona morta de +-45 para evitar tremor quando o joystick está solto
    if (abs(erro) > 45) {
      // Mapeia a inclinação do joystick para velocidade suave (0.2 a 2.5 graus por ciclo)
      float incremento = map(erro, -512, 512, -25, 25) / 10.0;
      posAtual[indiceServo] += incremento;

      // Trava nos limites físicos configurados
      posAtual[indiceServo] = constrain(posAtual[indiceServo], SERVO_ANG_MIN, SERVO_ANG_MAX);
      bracoServos[indiceServo].write((int)posAtual[indiceServo]);
    }
  }
}

// ============================================================================
// 7. MEMÓRIA EEPROM E BOTÕES DE CONTROLE
// ============================================================================
void verificarBotoes() {
  // Botão de troca de camada (Servos 1-4 <-> Servos 5-8)
  if (digitalRead(BTN_LAYER) == LOW && qtdServosAtivos > 4) {
    camadaSuperior = !camadaSuperior;
    Serial.print(F(">> CAMADA ALTERADA: Controlando Servos "));
    Serial.println(camadaSuperior ? F("5 a 8") : F("1 a 4"));
    delay(300); // Debounce simples
  }

  // Botão Salvar Posição na EEPROM
  if (digitalRead(BTN_SAVE) == LOW) {
    delay(300);
    if (digitalRead(BTN_SAVE) == LOW) { // Confirma pressão intencional
      salvarPassoEEPROM();
      // Se segurar por mais 1.5s, limpa a memória
      delay(1500);
      if (digitalRead(BTN_SAVE) == LOW) {
        totalPassosSalvos = 0;
        EEPROM.write(0, 0);
        Serial.println(F(">> MEMORIA EEPROM LIMPA!"));
        delay(500);
      }
    }
  }

  // Botão Reproduzir Sequência da EEPROM
  if (digitalRead(BTN_PLAY) == LOW && totalPassosSalvos > 0) {
    delay(300);
    executarSequenciaEEPROM();
  }
}

void salvarPassoEEPROM() {
  if (totalPassosSalvos >= MAX_PASSOS) {
    Serial.println(F(">> ERRO: Memoria cheia (Max 15 passos). Segure SAVE para limpar."));
    return;
  }

  int enderecoBase = 1 + (totalPassosSalvos * 8);
  for (int i = 0; i < 8; i++) {
    EEPROM.write(enderecoBase + i, (byte)posAtual[i]);
  }
  
  totalPassosSalvos++;
  EEPROM.write(0, totalPassosSalvos);
  
  Serial.print(F(">> POSICAO GRAVADA NA EEPROM! Passo: "));
  Serial.println(totalPassosSalvos);
}

void executarSequenciaEEPROM() {
  Serial.println(F(">> EXECUTANDO SEQUENCIA GRAVADA..."));
  for (int passo = 0; passo < totalPassosSalvos; passo++) {
    int enderecoBase = 1 + (passo * 8);
    
    // Move suavemente até a posição gravada
    bool emMovimento = true;
    while (emMovimento) {
      emMovimento = false;
      for (int i = 0; i < qtdServosAtivos; i++) {
        int alvo = EEPROM.read(enderecoBase + i);
        if (abs(posAtual[i] - alvo) > 1.0) {
          posAtual[i] += (alvo > posAtual[i]) ? 1.0 : -1.0;
          bracoServos[i].write((int)posAtual[i]);
          emMovimento = true;
        }
      }
      delay(15); // Velocidade da reprodução
    }
    delay(400); // Pausa entre pontos
  }
  Serial.println(F(">> SEQUENCIA CONCLUIDA."));
}

// ============================================================================
// 8. TELEMETRIA SERIAL FORMATADA
// ============================================================================
void imprimirCabecalhoSistema() {
  Serial.println(F("=================================================="));
  Serial.print(F("SISTEMA ATIVO: ")); Serial.print(qtdServosAtivos); Serial.println(F(" SERVOS"));
  Serial.print(F("SPEC -> servo:")); Serial.print(SERVO_MODELO);
  Serial.print(F(" angulo:")); Serial.print(SERVO_ANG_MIN); Serial.print(F("-")); Serial.print(SERVO_ANG_MAX);
  Serial.print(F(" forca:")); Serial.print(SERVO_FORCA_GRAMAS); Serial.println(F("g"));
  Serial.print(F("PASSOS NA MEMORIA: ")); Serial.println(totalPassosSalvos);
  Serial.println(F("=================================================="));
}

void enviarTelemetriaSerial() {
  float alturaZ, raioAtual, raioMax;
  calcularCinematica(alturaZ, raioAtual, raioMax);

  // Linha 1: Especificação e Domo de Operação
  Serial.print(F("servo:")); Serial.print(SERVO_MODELO);
  Serial.print(F(" angulo:")); Serial.print(SERVO_ANG_MIN); Serial.print(F("-")); Serial.print(SERVO_ANG_MAX);
  Serial.print(F(" forca:")); Serial.print(SERVO_FORCA_GRAMAS); Serial.print(F("g | "));
  
  Serial.print(F("Altura:")); Serial.print(alturaZ, 1); Serial.print(F("mm "));
  Serial.print(F("Raio:")); Serial.print(raioAtual, 1); Serial.print(F("/")); Serial.print(raioMax, 0); Serial.print(F("mm | "));

  // Linha 2 (contínua): Posição de cada servo ativo
  Serial.print(F("Pos["));
  for (int i = 0; i < qtdServosAtivos; i++) {
    Serial.print(F("S")); Serial.print(i + 1); Serial.print(F(":"));
    Serial.print((int)posAtual[i]);
    if (i < qtdServosAtivos - 1) Serial.print(F(" "));
  }
  Serial.println(F("]"));
}
