#include <WiFi.h>
#include <WebServer.h>
#include <math.h>

const char* ssid = "Air Quality Monitor";
const char* password = "12345678";

WebServer server(80);

int mq135Pin = 34;
int buzzerPin = 25;

// 🔥 Auto baseline
int baseValue = 0;

int getAQI(int sensorValue) {
  int diff = sensorValue - baseValue;
  if (diff < 0) diff = 0;

  int aqi = map(diff, 0, 1000, 0, 500);
  return constrain(aqi, 0, 500);
}

String getHTML(int aqi) {

  String status = "";
  String color = "";

  if (aqi <= 50) {
    status = "FRESH AIR";
    color = "green";
  } else if (aqi <= 100) {
    status = "NORMAL";
    color = "lightgreen";
  } else if (aqi <= 200) {
    status = "MODERATE";
    color = "orange";
  } else {
    status = "DANGEROUS";
    color = "red";
  }

  int angle = map(aqi, 0, 500, 180, 0);
  float rad = radians(angle);

  int cx = 150, cy = 150, r = 100;

  int x = cx + r * cos(rad);
  int y = cy - r * sin(rad);

  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name='viewport' content='width=device-width, initial-scale=1'>";
  
  html += "<style>";
  html += "body { text-align:center; font-family:Arial; background:#f2f2f2; margin:0; }";
  html += ".status { font-size:26px; font-weight:bold; color:" + color + "; }";
  html += "</style>";

  html += "</head><body>";
  html += "<h2>Real-Time Air Quality Monitor</h2>";

  html += "<svg width='320' height='180'>";
  html += "<defs><linearGradient id='grad'>";
  html += "<stop offset='0%' stop-color='green'/>";
  html += "<stop offset='50%' stop-color='yellow'/>";
  html += "<stop offset='100%' stop-color='red'/>";
  html += "</linearGradient></defs>";

  html += "<path d='M50 150 A100 100 0 0 1 250 150' fill='none' stroke='url(#grad)' stroke-width='20'/>";
  html += "<line x1='150' y1='150' x2='" + String(x) + "' y2='" + String(y) + "' stroke='black' stroke-width='4'/>";
  html += "</svg>";

  html += "<div class='status'>" + status + "</div>";
  html += "<h3>AQI Value: " + String(aqi) + "</h3>";

  html += "<script>setTimeout(function(){ location.reload(); }, 2000);</script>";
  html += "</body></html>";

  return html;
}

void handleRoot() {
  int sensorValue = analogRead(mq135Pin);
  int aqi = getAQI(sensorValue);

  // 🔊 BUZZER LOGIC
  if (aqi > 200) {
    digitalWrite(buzzerPin, HIGH);  // ON
  } else {
    digitalWrite(buzzerPin, LOW);   // OFF
  }

  Serial.print("AQI: ");
  Serial.println(aqi);

  server.send(200, "text/html", getHTML(aqi));
}

void setup() {
  Serial.begin(115200);

  pinMode(buzzerPin, OUTPUT);

  // 🔥 BASELINE CALIBRATION
  delay(3000);
  int sum = 0;
  for (int i = 0; i < 20; i++) {
    sum += analogRead(mq135Pin);
    delay(100);
  }
  baseValue = sum / 20;

  Serial.print("Base Value: ");
  Serial.println(baseValue);

  WiFi.softAP(ssid, password);

  Serial.println("Hotspot Started");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
}
