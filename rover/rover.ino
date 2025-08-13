/******* WiFi Robot Remote Control Mode ********/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoOTA.h>

// connections for drive Motors using IN1, IN2, IN3, IN4
int IN1 = D1;
int IN2 = D2;
int IN3 = D3;
int IN4 = D4;

const int buzPin = D5;      // buzzer pin
const int ledPin = D8;      // LED pin
const int wifiLedPin = D0;  // Wifi status LED

String command;               // store command from app
ESP8266WebServer server(80);  // HTTP server

unsigned long previousMillis = 0;
String sta_ssid = "";      // STA SSID (leave blank for AP only)
String sta_password = "";  // STA Password

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("WiFi Robot Remote Control Mode");
  Serial.println("--------------------------------------");

  pinMode(buzPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(wifiLedPin, OUTPUT);
  digitalWrite(buzPin, LOW);
  digitalWrite(ledPin, LOW);
  digitalWrite(wifiLedPin, HIGH);

  // Set motor pins as outputs
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  Stop();  // Stop motors initially

  // Generate hostname
  String chip_id = String(ESP.getChipId(), HEX);
  chip_id = chip_id.substring(chip_id.length() - 4);
  String hostname = "wificar-" + chip_id;
  Serial.println("Hostname: " + hostname);

  // Connect as WiFi client first
  WiFi.mode(WIFI_STA);
  WiFi.begin(sta_ssid.c_str(), sta_password.c_str());
  Serial.print("Connecting to: ");
  Serial.println(sta_ssid);

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;
  while (WiFi.status() != WL_CONNECTED && currentMillis - previousMillis <= 10000) {
    delay(500);
    Serial.print(".");
    currentMillis = millis();
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi-STA-Mode");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    digitalWrite(wifiLedPin, LOW);
    delay(3000);
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostname.c_str());
    IPAddress myIP = WiFi.softAPIP();
    Serial.println("\nWiFi failed to connect to " + sta_ssid);
    Serial.println("WiFi-AP-Mode");
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    digitalWrite(wifiLedPin, HIGH);
    delay(3000);
  }

  // HTTP handlers
  server.on("/", HTTP_handleRoot);
  server.onNotFound(HTTP_handleRoot);
  server.begin();

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();

  command = server.arg("State");

  // Motor Controls
  if (command == "F") Forward();
  else if (command == "B") Backward();
  else if (command == "R") TurnRight();
  else if (command == "L") TurnLeft();
  else if (command == "G") ForwardLeft();
  else if (command == "H") BackwardLeft();
  else if (command == "I") ForwardRight();
  else if (command == "J") BackwardRight();
  else if (command == "S") Stop();

  // Buzzer Logic: ON only when command == "V"
  if (command == "V") {
    digitalWrite(buzPin, HIGH);
  } else {
    digitalWrite(buzPin, LOW);
  }

  // LED Controls
  if (command == "W") TurnLightOn();
  else if (command == "w") TurnLightOff();
}

void HTTP_handleRoot() {
  server.send(200, "text/html", "");
  if (server.hasArg("State")) {
    Serial.println(server.arg("State"));
  }
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

// === Motor Control Functions (Digital Only) ===
void Forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void Backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void TurnRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void TurnLeft() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void ForwardLeft() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void BackwardLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void ForwardRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void BackwardRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void Stop() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// === Auxiliary Functions ===
void TurnLightOn() {
  digitalWrite(ledPin, HIGH);
}

void TurnLightOff() {
  digitalWrite(ledPin, LOW);
}
