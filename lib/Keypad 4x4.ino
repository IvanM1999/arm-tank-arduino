#include <Keypad.h>

const byte LINHAS = 4; // Quatro linhas
const byte COLUNAS = 4; // Quatro colunas

// Definição do mapa de teclas
char mapaTeclas[LINHAS][COLUNAS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Conecte os pinos do teclado a essas portas digitais do Arduino
byte pinosLinhas[LINHAS] = {9, 8, 7, 6}; 
byte pinosColunas[COLUNAS] = {5, 4, 3, 2}; 

Keypad teclado = Keypad(makeKeymap(mapaTeclas), pinosLinhas, pinosColunas, LINHAS, COLUNAS);

void setup() {
  Serial.begin(9600);
  Serial.println("--- Teste de Teclado Matricial 4x4 ---");
  Serial.println("Pressione qualquer tecla...");
}

void loop() {
  char tecla = teclado.getKey();
  
  if (tecla) {
    Serial.print("Tecla Pressionada: ");
    Serial.println(tecla);
  }
}
