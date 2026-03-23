#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11
#define MQ135_PIN 34

const char* ssid = "AirQuality_MonitoringSystem";
const char* password = "12345678";

// ✅ Custom valid IP
IPAddress local_IP(192,168,10,10);
IPAddress gateway(192,168,10,10);
IPAddress subnet(255,255,255,0);

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

float temperature, humidity;
int airQuality;

void handleRoot() {
  String html = R"rawliteral(
  <!DOCTYPE html>
  <html>
  <head>
    <title>Air Quality Monitor</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <meta http-equiv="refresh" content="2">
    <style>
      body {
        font-family: Arial;
        text-align: center;
        background: linear-gradient(to right, #4facfe, #00f2fe);
        color: white;
      }
      .card {
        background: rgba(255,255,255,0.2);
        padding: 20px;
        margin: 20px auto;
        width: 250px;
        border-radius: 15px;
        box-shadow: 0 4px 10px rgba(0,0,0,0.3);
      }
      h2 {
        margin-top: 30px;
      }
      .value {
        font-size: 28px;
        font-weight: bold;
      }
    </style>
  </head>
  <body>
    <h2> Air Quality Monitoring</h2>

    <div class="card">
      <h3> Temperature</h3>
      <div class="value">)rawliteral" + String(temperature) + R"rawliteral( °C</div>
    </div>

    <div class="card">
      <h3>Humidity</h3>
      <div class="value">)rawliteral" + String(humidity) + R"rawliteral( %</div>
    </div>

    <div class="card">
      <h3> Air Quality</h3>
      <div class="value">)rawliteral" + String(airQuality) + R"rawliteral(</div>
    </div>

  </body>
  </html>
  )rawliteral";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // ✅ Set custom IP
  WiFi.softAPConfig(local_IP, local_IP, subnet);

  // Hotspot start
  WiFi.softAP(ssid, password);

  Serial.println("WiFi Hotspot Started!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  airQuality = analogRead(MQ135_PIN);

  server.handleClient();
}
