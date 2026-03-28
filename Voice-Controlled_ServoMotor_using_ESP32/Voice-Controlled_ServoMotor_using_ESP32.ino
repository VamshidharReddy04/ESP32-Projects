#include "BluetoothSerial.h"
#include <ESP32Servo.h>

BluetoothSerial SerialBT;
Servo myServo;

int servoPin = 13;
int ledPin = 12;   // NEW LED PIN

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Servo_Control");

  Serial.println("Bluetooth Started. Waiting for commands...");

  myServo.attach(servoPin);
  myServo.write(0); // initial position

  pinMode(ledPin, OUTPUT);   // LED setup
  digitalWrite(ledPin, LOW); // LED OFF initially
}

void loop() {
  if (SerialBT.available()) {

    String command = SerialBT.readString();
    command.trim();
    command.toLowerCase();

    Serial.print("Received: ");
    Serial.println(command);

    if (command.indexOf("open") >= 0) {
      myServo.write(90);
      digitalWrite(ledPin, HIGH);   // LED ON
      Serial.println("Servo Opened + LED ON");
    } 
    else if (command.indexOf("close") >= 0) {
      myServo.write(0);
      digitalWrite(ledPin, LOW);    // LED OFF
      Serial.println("Servo Closed + LED OFF");
    } 
    else {
      Serial.println("Unknown Command");
    }
  }
}
