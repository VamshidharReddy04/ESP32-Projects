#include <WiFi.h>
#include <ESP32Servo.h>

const char* ssid = "Rain_Servomotor";
const char* password = "12345678";

Servo myServo;

int rainPin = 34;
int buzzer = 19;

WiFiServer server(80);

int servoPos = 0;

// ⏱ Timing control
bool triggered = false;
unsigned long triggerTime = 0;

const int buzzerTime = 3000;   // 3 sec
const int cooldownTime = 8000; // 8 sec

void setup() {
  Serial.begin(115200);

  pinMode(rainPin, INPUT);
  pinMode(buzzer, OUTPUT);

  myServo.attach(18);
  myServo.write(0); // initially OPEN

  WiFi.softAP(ssid, password);

  Serial.println("WiFi Started");
  Serial.println(WiFi.softAPIP());

  server.begin();
}

void loop() {

  int rain = digitalRead(rainPin);
  unsigned long currentTime = millis();

  // 🌧 RAIN TRIGGER (ONLY ONCE)
  if (rain == LOW && !triggered) {

    Serial.println("Rain Detected");

    // 🔔 Buzzer ON
    digitalWrite(buzzer, HIGH);

    // 🔄 Servo CLOSE (only once)
    myServo.write(90);
    servoPos = 90;

    triggered = true;
    triggerTime = currentTime;
  }

  // 🔔 Buzzer OFF after 3 sec
  if (triggered && (currentTime - triggerTime >= buzzerTime)) {
    digitalWrite(buzzer, LOW);
  }

  // 🔁 Reset after 8 sec
  if (triggered && (currentTime - triggerTime >= cooldownTime)) {
    Serial.println("Ready for next rain trigger");
    triggered = false;
  }

  // 🌐 WEB SERVER
  WiFiClient client = server.available();

  if (client) {

    String request = client.readStringUntil('\r');
    client.flush();

    // 🔘 MANUAL CONTROL
    if (request.indexOf("/open") != -1) {
      myServo.write(0);
      servoPos = 0;
    }

    if (request.indexOf("/close") != -1) {
      myServo.write(90);
      servoPos = 90;
    }

    // 📡 STATUS API
    if (request.indexOf("/status") != -1) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: application/json");
      client.println("Connection: close");
      client.println();

      client.print("{\"rain\":");
      client.print(rain);
      client.print(",\"servo\":");
      client.print(servoPos);
      client.print("}");

      delay(5);
      client.stop();
      return;
    }

    // 🌐 WEB PAGE
    client.println("HTTP/1.1 200 OK");
    client.println("Content-type:text/html; charset=UTF-8");
    client.println("Connection: close");
    client.println();

    client.println(R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<meta charset="UTF-8">

<style>
body {
  font-family: Arial;
  text-align: center;
  background: linear-gradient(to right, #36d1dc, #5b86e5);
  color: white;
}
.card {
  background: white;
  color: black;
  padding: 20px;
  margin: 20px;
  border-radius: 15px;
}
button {
  padding: 15px 25px;
  margin: 10px;
  font-size: 18px;
  border-radius: 10px;
  border: none;
}
.open { background: green; color: white; }
.close { background: red; color: white; }
</style>
</head>

<body>

<h1>Rain Monitor System</h1>

<div class="card">
  <h2 id="rain">Checking...</h2>
  <p id="servo">Servomotor </p>

  <button class="open" onclick="sendCmd('/open')">OPEN</button>
  <button class="close" onclick="sendCmd('/close')">CLOSE</button>
</div>

<script>

function sendCmd(cmd){
  fetch(cmd).catch(()=>{});
}

function updateStatus(){
  fetch('/status')
  .then(res => res.json())
  .then(data => {

    document.getElementById("rain").innerHTML =
      (data.rain == 0) ? "Rain Started" : "No Rain";

    document.getElementById("servo").innerHTML =
      (data.servo == 90) ? "Closed" : "Open";

  })
  .catch(() => {
    document.getElementById("rain").innerHTML = "Connecting...";
    document.getElementById("servo").innerHTML = "Servomotor";
  });
}

setInterval(updateStatus, 3000);
updateStatus();

</script>

</body>
</html>
)rawliteral");

    client.stop();
  }
}
