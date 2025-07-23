#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// WiFi
#define WIFI_SSID "YourWiFiSSID"
#define WIFI_PASSWORD "YourWiFiPassword"

// Firebase
#define FIREBASE_HOST "smart-care-hub-7aa4b-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "your_firebase_database_secret"

// User ID from your app
#define USER_ID "123456789"

// OLED Setup
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Firebase object
FirebaseData firebaseData;

// Reed switch pins
const int reedPin1 = D3; // GPIO0
const int reedPin2 = D5; // GPIO14
const int reedPin3 = D6; // GPIO12

// State flags
bool med1Taken = false;
bool med2Taken = false;
bool med3Taken = false;

void setup() {
  Serial.begin(115200);

  // OLED display init
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Connecting WiFi...");
  display.display();

  // Reed pin setup
  pinMode(reedPin1, INPUT);
  pinMode(reedPin2, INPUT);
  pinMode(reedPin3, INPUT);

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }

  display.println("WiFi OK");
  display.display();

  // Connect to Firebase
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);
  display.println("Firebase OK");
  display.display();
  delay(1000);
}

void loop() {
  // Check reed switch states (LOW = activated)
  bool opened1 = digitalRead(reedPin1) == LOW;
  bool opened2 = digitalRead(reedPin2) == LOW;
  bool opened3 = digitalRead(reedPin3) == LOW;

  if (opened1 && !med1Taken) {
    med1Taken = true;
    updateFirebase("med 1");
  }

  if (opened2 && !med2Taken) {
    med2Taken = true;
    updateFirebase("med 2");
  }

  if (opened3 && !med3Taken) {
    med3Taken = true;
    updateFirebase("med 3");
  }

  updateDisplay();
  delay(500);
}

void updateFirebase(String medSlot) {
  String path = "/users/" + String(USER_ID) + "/medicines/" + medSlot + "/taken";
  if (Firebase.setBool(firebaseData, path, true)) {
    Serial.println("Updated: " + medSlot + " = true");
  } else {
    Serial.print("Error: ");
    Serial.println(firebaseData.errorReason());
  }
}

void updateDisplay() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Medicine Taken:");
  display.println("Med 1: " + String(med1Taken ? "Yes" : "No"));
  display.println("Med 2: " + String(med2Taken ? "Yes" : "No"));
  display.println("Med 3: " + String(med3Taken ? "Yes" : "No"));
  display.display();
}
