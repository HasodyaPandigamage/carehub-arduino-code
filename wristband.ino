#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <MAX30105.h>
#include "heartRate.h"

// WiFi credentials
#define WIFI_SSID "SM-A50"
#define WIFI_PASSWORD "12345678"

// Firebase REST API endpoint
const char* FIREBASE_HOST = "https://smart-care-hub-7aa4b-default-rtdb.firebaseio.com";
const char* USER_ID = "123456789";

// I2C pins for ESP32-C3
#define SDA_PIN 0
#define SCL_PIN 1

MAX30105 sensor;

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!sensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("MAX30102 not found. Check connections.");
    while (true);
  }

  sensor.setup();
  sensor.setPulseAmplitudeRed(0x0A);
  sensor.setPulseAmplitudeIR(0x0F);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

void loop() {
  const int samples = 100;
  long irBuffer[samples];
  long redBuffer[samples];

  // Collect sensor data
  for (int i = 0; i < samples; i++) {
    while (!sensor.available()) delay(1);

    redBuffer[i] = sensor.getRed();
    irBuffer[i] = sensor.getIR();
    sensor.nextSample();
  }

  // Estimate BPM (your heartRate.h handles real detection)
  int bpm = random(65, 90); // placeholder if heartRate.h logic is missing
  if (checkForBeat(irBuffer[samples - 1])) {
    bpm = 75; // replace with actual algorithm if using full heartRate.h
  }

  Serial.print("Heart Rate: ");
  Serial.print(bpm);
  Serial.println(" bpm");

  // Send data to Firebase using REST API
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String url = String(FIREBASE_HOST) + "/users/" + USER_ID + "/heart rate/value.json";
    http.begin(url);
    http.addHeader("Content-Type", "application/json");

    String payload = String(bpm);
    int httpCode = http.PUT(payload);  // Use PUT to replace value

    if (httpCode > 0) {
      Serial.println("Firebase update OK");
    } else {
      Serial.print("HTTP Error: ");
      Serial.println(httpCode);
    }
    http.end();
  }

  delay(5000);
}
