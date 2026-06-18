#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_AHTX0.h>
#include <Wire.h>

// --- Configurações da Rede Wi-Fi ---
const char* ssid = "Robo_Tank_Lab";
const char* password = "12345678password";

// --- Definições de Pinos ---
#define PINO_OSCILOSCOPIO 34 // Entrada Analógica (ADC1_CH6)

// --- Inicialização de Objetos ---
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
Adafruit_AHTX0 aht;

// Variáveis para temporização das leituras
unsigned long tempoAnteriorSensor = 0;
const long intervaloSensor = 2000; // Ler temperatura a cada 2 segundos

// --- Interface HTML Embutida (String) ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Laboratório Móvel ESP32</title>
    <style>
        body { font-family: Arial, sans-serif; background-color: #1e1e24; color: #f0f0f5; text-align: center; margin: 0; padding: 20px; }
        .container { max-width: 800px; margin: 0 auto; }
        .card { background-color: #2a2a35; padding: 20px; border-radius: 10px; margin-bottom: 20px; box-shadow: 0 4px 8px rgba(0,0,0,0.2); }
        .grid { display: grid; grid-template-columns: 1fr 1fr; gap: 20px; }
        h1 { color: #4A90E2; }
        .leitura { font-size: 24px; font-weight: bold; color: #00E676; }
        canvas { background-color: #000; border: 2px solid #4A90E2; border-radius: 5px; width: 100%; height: 300px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Ecossistema Robótico - Laboratório & Osciloscópio</h1>
        
        <div class="card grid">
            <div>
                <h3>Temperatura (AHT10)</h3>
                <div id="temp" class="leitura">-- °C</div>
            </div>
            <div>
                <h3>Humidade (AHT10)</h3>
                <div id="hum" class="leitura">-- %</div>
            </div>
        </div>

        <div class="card">
            <h3>Osciloscópio Digital (Pino 34)</h3>
            <canvas id="canvasOsc"></canvas>
        </div>
    </div>

    <script>
        var gateway = `ws://${window.location.hostname}/ws`;
        var websocket;
        
        // Configuração do Gráfico do Osciloscópio (Canvas)
        var canvas = document.getElementById('canvasOsc');
        var ctx = canvas.getContext('2d');
        var pontos = new Array(200).fill(150); // Buffer visual

        function initWebSocket() {
            websocket = new WebSocket(gateway);
            websocket.onmessage = onMessage;
        }

        function onMessage(event) {
            var data = JSON.parse(event.data);
            
            // Se receber dados do laboratório (AHT10)
            if(data.type === "lab") {
                document.getElementById('temp').innerText = data.temp.toFixed(1) + " °C";
                document.getElementById('hum').innerText = data.hum.toFixed(1) + " %";
            }
            
            // Se receber dados do osciloscópio (ADC)
            if(data.type === "osc") {
                pontos.push(data.val);
                if(pontos.length > canvas.width) pontos.shift();
                desenharOsciloscopio();
            }
        }

        function desenharOsciloscopio() {
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            
            // Desenha a grelha de fundo
            ctx.strokeStyle = '#333';
            ctx.lineWidth = 1;
            for(let i=0; i<canvas.width; i+=40) {
                ctx.beginPath(); ctx.moveTo(i, 0); ctx.lineTo(i, canvas.height); ctx.stroke();
            }
            for(let j=0; j<canvas.height; j+=40) {
                ctx.beginPath(); ctx.moveTo(0, j); ctx.lineTo(canvas.width, j); ctx.stroke();
            }

            // Desenha a linha do sinal
            ctx.strokeStyle = '#00FF00';
            ctx.lineWidth = 2;
            ctx.beginPath();
            for(let x=0; x<pontos.length; x++) {
                // Mapeia o valor de 0-4095 para a altura do canvas
                let y = canvas.height - (pontos[x] / 4095 * canvas.height);
                if(x === 0) ctx.moveTo(x, y);
                else ctx.lineTo(x, y);
            }
            ctx.stroke();
        }

        window.onload = function() {
            canvas.width = canvas.parentElement.clientWidth - 40;
            canvas.height = 250;
            initWebSocket();
        };
    </script>
</body>
</html>
)rawliteral";

// --- Gestão de Eventos do WebSocket ---
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  // Tratamento de conexões se necessário no futuro (ex: app Java a conectar-se)
}

void setup() {
  Serial.begin(115200);
  
  // Inicializa o Sensor AHT10
  if (!aht.begin()) {
    Serial.println("Aviso: Sensor AHT10 não encontrado! Verifique as conexões I2C.");
  } else {
    Serial.println("Sensor AHT10 inicializado com sucesso.");
  }

  // Configura o ESP32 como Access Point (Ponto de Acesso Wi-Fi)
  WiFi.softAP(ssid, password);
  Serial.print("Rede Wi-Fi criada. IP do Servidor: ");
  Serial.println(WiFi.softAPIP());

  // Configura os WebSockets e as rotas do servidor
  ws.onEvent(onEvent);
  server.addHandler(&ws);

  // Rota principal que entrega a interface HTML
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.begin();
  Serial.println("Servidor Web em execução!");
}

void loop() {
  ws.cleanupClients(); // Limpa conexões WebSocket perdidas

  // 1. LEITURA ULTRA RÁPIDA DO OSCILOSCÓPIO (ADC)
  int valorAnalogico = analogRead(PINO_OSCILOSCOPIO);
  
  // Envia a leitura imediatamente via WebSocket se houver clientes conectados
  if (ws.count() > 0) {
    String jsonOsc = "{\"type\":\"osc\",\"val\":" + String(valorAnalogico) + "}";
    ws.textAll(jsonOsc);
  }

  // 2. LEITURA TEMPORIZADA DO LABORATÓRIO (AHT10)
  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnteriorSensor >= intervaloSensor) {
    tempoAnteriorSensor = tempoAtual;

    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp); // Executa a leitura física do sensor

    if (ws.count() > 0) {
      String jsonLab = "{\"type\":\"lab\",\"temp\":" + String(temp.temperature) + ",\"hum\":" + String(humidity.relative_humidity) + "}";
      ws.textAll(jsonLab);
    }
  }
  
  // Pequeno delay para não sobrecarregar o núcleo do processador
  delayMicroseconds(500); 
}
