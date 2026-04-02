#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* ssid = "ESP32_Rain_Control";
const char* password = "12345678";

WebServer server(80);
Servo myServo;

int servoPin = 13;
int rainPin = 34;

int servoAngle = 0;
bool isRaining = false;

// Custom IP Configuration
IPAddress local_ip(192,168,10,10);
IPAddress gateway(192,168,10,10);
IPAddress subnet(255,255,255,0);

// HTML Page
String webpage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<title>ESP32 Smart Dashboard</title>
<meta name="viewport" content="width=device-width, initial-scale=1">

<style>
body {
  margin: 0;
  font-family: 'Segoe UI', sans-serif;
  background: linear-gradient(135deg, #667eea, #764ba2);
  color: white;
  text-align: center;
}

.card {
  background: rgba(255,255,255,0.1);
  backdrop-filter: blur(10px);
  margin: 40px auto;
  padding: 25px;
  border-radius: 20px;
  width: 90%;
  max-width: 350px;
  box-shadow: 0 8px 32px rgba(0,0,0,0.3);
}

h2 {
  margin-bottom: 10px;
}

.angle-box {
  font-size: 28px;
  font-weight: bold;
  background: rgba(255,255,255,0.2);
  padding: 10px;
  border-radius: 10px;
  margin: 15px 0;
}

input[type=range] {
  -webkit-appearance: none;
  width: 100%;
  height: 8px;
  border-radius: 5px;
  background: #ddd;
  outline: none;
}

input[type=range]::-webkit-slider-thumb {
  -webkit-appearance: none;
  width: 22px;
  height: 22px;
  background: #00ffcc;
  border-radius: 50%;
  cursor: pointer;
  box-shadow: 0 0 10px #00ffcc;
}

.btn {
  padding: 12px;
  margin: 10px 5px;
  border: none;
  border-radius: 12px;
  width: 45%;
  font-size: 16px;
  cursor: pointer;
  transition: 0.3s;
}

.random {
  background: #00c6ff;
  color: white;
}

.reset {
  background: #ff4b5c;
  color: white;
}

.btn:hover {
  transform: scale(1.05);
}

.footer {
  margin-top: 15px;
  font-size: 12px;
  opacity: 0.7;
}
</style>
</head>

<body>

<div class="card">
  <h2>Servo Dashboard</h2>

  <div class="angle-box">
    <span id="angleValue">0</span>°
  </div>

  <input type="range" min="0" max="180" value="0" id="slider"
  oninput="updateAngle(this.value)">

  <div>
    <button class="btn random" onclick="randomAngle()">Random</button>
    <button class="btn reset" onclick="resetAngle()"> Reset</button>
  </div>

  <div class="footer">
    ESP32 Smart Control
  </div>
</div>

<script>
function updateAngle(val) {
  document.getElementById("angleValue").innerHTML = val;
  fetch("/set?angle=" + val);
}

function randomAngle() {
  let rand = Math.floor(Math.random() * 181);
  document.getElementById("slider").value = rand;
  updateAngle(rand);
}

function resetAngle() {
  document.getElementById("slider").value = 0;
  updateAngle(0);
}
</script>

</body>
</html>
)rawliteral";

// Root Page
void handleRoot() {
  server.send(200, "text/html", webpage);
}

// Servo Control Route
void handleServo() {
  if (server.hasArg("angle")) {
    servoAngle = server.arg("angle").toInt();
    myServo.write(servoAngle);
  }
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  pinMode(rainPin, INPUT);
  myServo.attach(servoPin);

  // Set Custom IP
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password);

  Serial.println("WiFi Started");
  Serial.print("Open: http://");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/set", handleServo);

  server.begin();
}

void loop() {
  server.handleClient();

  int rainValue = digitalRead(rainPin);

  // Rain Logic
  if (rainValue == LOW) {
    if (!isRaining) {
      Serial.println("Rain Detected!");
      myServo.write(180);
      isRaining = true;
    }
  } else {
    if (isRaining) {
      Serial.println("No Rain");
      myServo.write(0);
      isRaining = false;
    }
  }

  delay(200);
}
