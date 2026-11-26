#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>
#include <Preferences.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <math.h>

// ============================================================================
// 1. SELEÇÃO AUTOMÁTICA DE PINOS (ESP32-C3 vs ESP32 DEVKIT V1)
// ============================================================================
#if CONFIG_IDF_TARGET_ESP32C3
  const int PINO_SERVOS[8] = {0, 1, 2, 3, 4, 5, 6, 7};
  const int PINO_SENSE_6 = 8;
  const int PINO_SENSE_8 = 10;
  const char* PLACA_NOME = "ESP32-C3";
#else
  const int PINO_SERVOS[8] = {13, 12, 14, 27, 26, 25, 33, 32};
  const int PINO_SENSE_6 = 18;
  const int PINO_SENSE_8 = 19;
  const char* PLACA_NOME = "ESP32-DevKit-V1";
#endif

// ============================================================================
// 2. CONFIGURAÇÕES DO SERVO E GEOMETRIA DO DOMO
// ============================================================================
const char* SERVO_MODELO = "MG90S_Metal";
const int SERVO_ANG_MIN = 0;
const int SERVO_ANG_MAX = 180;
const int SERVO_FORCA_GRAMAS = 2200; // 2.2kg.cm

const float L1_BASE_ALTURA = 55.0;  // mm
const float L2_BRACO = 110.0;       // mm
const float L3_ANTEBRACO = 110.0;   // mm

// Credenciais Wi-Fi (Modo Ponto de Acesso direto no Smartphone)
const char* AP_SSID = "Garra_Robotica_AP";
const char* AP_PASS = "12345678";

// UUIDs para Controle via Bluetooth Low Energy (BLE)
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

// ============================================================================
// 3. OBJETOS E VARIÁVEIS GLOBAIS
// ============================================================================
Servo bracoServos[8];
WebServer server(80);
Preferences memoriaNVS; // Substitui EEPROM com maior durabilidade no ESP32

int qtdServosAtivos = 4;
int posAtual[8] = {90, 90, 90, 90, 90, 90, 90, 90};
const int MAX_PASSOS = 15;
int totalPassosSalvos = 0;
unsigned long ultimoTempoSerial = 0;

// ============================================================================
// 4. INTERFACE WEB EMBARCADA (HTML + CSS + JS PARA SMARTPHONE)
// ============================================================================
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Controle Garra ESP32</title>
  <style>
    body { font-family: system-ui, sans-serif; background: #0d1117; color: #e6edf3; margin: 0; padding: 16px; }
    h2 { margin: 0 0 8px; font-size: 1.2rem; color: #58a6ff; }
    .telemetry { background: #161b22; border: 1px solid #30363d; padding: 12px; border-radius: 8px; font-family: monospace; font-size: 0.82rem; margin-bottom: 16px; color: #3fb950; }
    .card { background: #161b22; border: 1px solid #30363d; padding: 12px 14px; border-radius: 8px; margin-bottom: 10px; }
    .row { display: flex; justify-content: space-between; font-size: 0.9rem; margin-bottom: 6px; }
    input[type=range] { width: 100%; height: 28px; accent-color: #58a6ff; }
    .btn-grid { display: grid; grid-template-columns: 1fr 1fr; gap: 10px; margin-top: 16px; }
    button { padding: 14px; border: none; border-radius: 8px; font-weight: bold; font-size: 0.95rem; cursor: pointer; color: #fff; }
    .btn-save { background: #238636; }
    .btn-play { background: #1f6feb; }
    .btn-home { background: #d29922; color: #000; }
    .btn-clear { background: #da3633; }
  </style>
</head>
<body>
  <h2>🤖 Painel Garra Robótica (Wi-Fi / BLE)</h2>
  <div class="telemetry" id="telemetria">Conectando ao ESP32...</div>
  <div id="controles"></div>

  <div class="btn-grid">
    <button class="btn-save" onclick="cmd('save')">💾 Gravar Ponto</button>
    <button class="btn-play" onclick="cmd('play')">▶️ Executar Seq.</button>
    <button class="btn-home" onclick="cmd('home')">🎯 Reboot 90°</button>
    <button class="btn-clear" onclick="cmd('clear')">🗑️ Limpar Memória</button>
  </div>

  <script>
    const nomes = ["Base (S1)", "Ombro (S2)", "Cotovelo (S3)", "Garra (S4)", "Punho Pitch (S5)", "Punho Roll (S6)", "Aux 1 (S7)", "Aux 2 (S8)"];
    let qtdAtivos = 4;

    function initUI(qtd, posicoes) {
      qtdAtivos = qtd;
      const container = document.getElementById('controles');
      container.innerHTML = '';
      for (let i = 0; i < qtd; i++) {
        container.innerHTML += `
          <div class="card">
            <div class="row"><span>${nomes[i]}</span><strong id="val${i}">${posicoes[i]}°</strong></div>
            <input type="range" min="0" max="180" value="${posicoes[i]}" oninput="moverServo(${i}, this.value)">
          </div>`;
      }
    }

    let timeoutEnvio;
    function moverServo(id, ang) {
      document.getElementById('val' + id).innerText = ang + '°';
      clearTimeout(timeoutEnvio);
      timeoutEnvio = setTimeout(() => {
        fetch(`/set?s=${id}&a=${ang}`).then(r => r.text()).then(atualizarTexto);
      }, 25);
    }

    function cmd(acao) {
      fetch(`/cmd?c=${acao}`).then(r => r.text()).then(t => {
        atualizarTexto(t);
        if (acao === 'home' || acao === 'play') carregarEstado();
      });
    }

    function atualizarTexto(txt) {
      document.getElementById('telemetria').innerText = txt;
    }

    function carregarEstado() {
      fetch('/status').then(r => r.json()).then(data => {
        initUI(data.qtd, data.pos);
        atualizarTexto(data.telemetria);
      });
    }

    window.onload = carregarEstado;
  </script>
</body>
</html>
)rawliteral";

// ============================================================================
// 5. CALLBACK BLUETOOTH BLE (RECEBE COMANDOS EX: "S0:120" OU "HOME")
// ============================================================================
class MeuBLECallback: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue().c_str();
    rxValue.trim();
    if (rxValue.length() > 0) {
      Serial.print("[BLE RX]: "); Serial.println(rxValue);
      if (rxValue == "HOME") {
        resetarPara90();
      } else if (rxValue.startsWith("S")) {
        int sep = rxValue.indexOf(':');
        if (sep > 0) {
          int idx = rxValue.substring(1, sep).toInt();
          int ang = constrain(rxValue.substring(sep + 1).toInt(), SERVO_ANG_MIN, SERVO_ANG_MAX);
          if (idx >= 0 && idx < qtdServosAtivos) {
            posAtual[idx] = ang;
            bracoServos[idx].write(ang);
          }
        }
      }
    }
  }
};

// ============================================================================
// 6. SETUP: HARDWARE, WI-FI AP, SERVIDOR WEB E BLUETOOTH BLE
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PINO_SENSE_6, INPUT_PULLUP);
  pinMode(PINO_SENSE_8, INPUT_PULLUP);

  // Detecta 4, 6 ou 8 servos
  if (digitalRead(PINO_SENSE_8) == LOW) qtdServosAtivos = 8;
  else if (digitalRead(PINO_SENSE_6) == LOW) qtdServosAtivos = 6;
  else qtdServosAtivos = 4;

  // Inicializa memória Flash NVS
  memoriaNVS.begin("garra_robot", false);
  totalPassosSalvos = memoriaNVS.getInt("total", 0);

  // Inicializa Servos com Soft-Start em 90 graus
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  for (int i = 0; i < qtdServosAtivos; i++) {
    bracoServos[i].setPeriodHertz(50);
    bracoServos[i].attach(PINO_SERVOS[i], 500, 2400);
    bracoServos[i].write(90);
    posAtual[i] = 90;
    delay(150);
  }

  // Ativa Placa de Rede Wi-Fi (Access Point)
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("Wi-Fi AP Ativo: "); Serial.println(AP_SSID);
  Serial.print("Acesse no Smartphone: http://"); Serial.println(WiFi.softAPIP());

  // Ativa Bluetooth Low Energy (BLE)
  BLEDevice::init("Garra_BLE_Control");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
  pCharacteristic->setCallbacks(new MeuBLECallback());
  pService->start();
  BLEDevice::getAdvertising()->start();
  Serial.println("Bluetooth BLE Ativo: Garra_BLE_Control");

  // Rotas do Servidor Web
  server.on("/", []() { server.send_P(200, "text/html", INDEX_HTML); });
  
  server.on("/status", []() {
    String json = "{\"qtd\":" + String(qtdServosAtivos) + ",\"pos\":[";
    for (int i = 0; i < qtdServosAtivos; i++) {
      json += String(posAtual[i]) + (i < qtdServosAtivos - 1 ? "," : "");
    }
    json += "],\"telemetria\":\"" + gerarStringTelemetria() + "\"}";
    server.send(200, "application/json", json);
  });

  server.on("/set", []() {
    if (server.hasArg("s") && server.hasArg("a")) {
      int s = server.arg("s").toInt();
      int a = constrain(server.arg("a").toInt(), SERVO_ANG_MIN, SERVO_ANG_MAX);
      if (s >= 0 && s < qtdServosAtivos) {
        posAtual[s] = a;
        bracoServos[s].write(a);
      }
    }
    server.send(200, "text/plain", gerarStringTelemetria());
  });

  server.on("/cmd", []() {
    String c = server.arg("c");
    if (c == "home") resetarPara90();
    else if (c == "save") salvarPassoNVS();
    else if (c == "play") executarSequenciaNVS();
    else if (c == "clear") {
      totalPassosSalvos = 0;
      memoriaNVS.putInt("total", 0);
    }
    server.send(200, "text/plain", gerarStringTelemetria());
  });

  server.begin();
}

// ============================================================================
// 7. LOOP PRINCIPAL
// ============================================================================
void loop() {
  server.handleClient();

  if (millis() - ultimoTempoSerial >= 500) {
    ultimoTempoSerial = millis();
    Serial.println(gerarStringTelemetria());
  }
}

// ============================================================================
// 8. FUNÇÕES DE CINEMÁTICA, MEMÓRIA NVS E TELEMETRIA
// ============================================================================
void resetarPara90() {
  for (int i = 0; i < qtdServosAtivos; i++) {
    posAtual[i] = 90;
    bracoServos[i].write(90);
    delay(80);
  }
}

void salvarPassoNVS() {
  if (totalPassosSalvos >= MAX_PASSOS) return;
  for (int i = 0; i < 8; i++) {
    String chave = "p" + String(totalPassosSalvos) + "s" + String(i);
    memoriaNVS.putInt(chave.c_str(), posAtual[i]);
  }
  totalPassosSalvos++;
  memoriaNVS.putInt("total", totalPassosSalvos);
}

void executarSequenciaNVS() {
  for (int p = 0; p < totalPassosSalvos; p++) {
    bool movendo = true;
    while (movendo) {
      movendo = false;
      for (int i = 0; i < qtdServosAtivos; i++) {
        String chave = "p" + String(p) + "s" + String(i);
        int alvo = memoriaNVS.getInt(chave.c_str(), 90);
        if (posAtual[i] != alvo) {
          posAtual[i] += (alvo > posAtual[i]) ? 1 : -1;
          bracoServos[i].write(posAtual[i]);
          movendo = true;
        }
      }
      delay(15);
    }
    delay(350);
  }
}

String gerarStringTelemetria() {
  float raioMax = L2_BRACO + L3_ANTEBRACO;
  float radOmbro = radians(posAtual[1]);
  float radCotovelo = radians(posAtual[2] - 90.0);

  float alturaZ = L1_BASE_ALTURA + (L2_BRACO * sin(radOmbro)) + (L3_ANTEBRACO * sin(radOmbro + radCotovelo));
  if (alturaZ < 0) alturaZ = 0;
  float raioAtual = abs((L2_BRACO * cos(radOmbro)) + (L3_ANTEBRACO * cos(radOmbro + radCotovelo)));

  String out = String(PLACA_NOME) + " | servo:" + String(SERVO_MODELO) + " ang:0-180 carga:" + String(SERVO_FORCA_GRAMAS) + "g | ";
  out += "Alt:" + String(alturaZ, 1) + "mm Domo:" + String(raioAtual, 1) + "/" + String(raioMax, 0) + "mm | Mem:" + String(totalPassosSalvos) + " | [";
  for (int i = 0; i < qtdServosAtivos; i++) {
    out += "S" + String(i + 1) + ":" + String(posAtual[i]) + (i < qtdServosAtivos - 1 ? " " : "");
  }
  out += "]";
  return out;
}
