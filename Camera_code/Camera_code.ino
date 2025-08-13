#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>

// Select camera model
#define CAMERA_MODEL_AI_THINKER

// Wi-Fi credentials
const char* ssid = "Note 9";
const char* password = "sahil123";

// GPIO settings for AI-Thinker
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// Motor pins
int gpLb =  2;   // Left Backward
int gpLf = 14;   // Left Forward
int gpRb = 15;   // Right Forward
int gpRf = 13;   // Right Backward
int gpLed =  4;  // Light
int gpFireMotor = 12; // 5th Motor

WebServer server(80);
String WiFiAddr = "";

// Declare existing function from app_httpd.cpp (don't redefine!)
void startCameraServer();

// 🔥 Fire motor handler
void handleFireMotor() {
  Serial.println("🔥 Fire button pressed!");
  digitalWrite(gpFireMotor, HIGH);
  delay(1000); // adjust if needed
  digitalWrite(gpFireMotor, LOW);
  server.send(200, "text/plain", "Fire motor activated!");
}

// 📷 Main UI handler
void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html><html>
<head>
  <title>ESP32-CAM Car</title>
  <style>
    body { background: #111; color: white; font-family: Arial; text-align: center; margin: 0; padding: 0; }
    h1 { margin: 20px 0; }
    .stream { max-width: 90vw; border-radius: 12px; box-shadow: 0 0 20px #000; }
    .fire-btn {
      margin-top: 25px;
      background: #ff4444;
      border: none;
      border-radius: 50%;
      width: 100px;
      height: 100px;
      font-size: 18px;
      color: white;
      box-shadow: 0 0 20px #ff2222;
      cursor: pointer;
      transition: transform 0.2s, box-shadow 0.3s;
    }
    .fire-btn:hover {
      transform: scale(1.1);
      box-shadow: 0 0 30px #ff0000;
    }
  </style>
</head>
<body>
  <h1>ESP32-CAM Car Controller</h1>
  <img class="stream" src="/stream" />
  <br>
  <button class="fire-btn" onclick="fetch('/fire').then(r => r.text()).then(alert)">FIRE</button>
</body>
</html>
)rawliteral";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  // Initialize all motor pins
  pinMode(gpLb, OUTPUT);
  pinMode(gpLf, OUTPUT);
  pinMode(gpRb, OUTPUT);
  pinMode(gpRf, OUTPUT);
  pinMode(gpLed, OUTPUT);
  pinMode(gpFireMotor, OUTPUT);

  // Reset states
  digitalWrite(gpLb, LOW);
  digitalWrite(gpLf, LOW);
  digitalWrite(gpRb, LOW);
  digitalWrite(gpRf, LOW);
  digitalWrite(gpLed, LOW);
  digitalWrite(gpFireMotor, LOW);

  // Camera config
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer   = LEDC_TIMER_0;
  config.pin_d0       = Y2_GPIO_NUM;
  config.pin_d1       = Y3_GPIO_NUM;
  config.pin_d2       = Y4_GPIO_NUM;
  config.pin_d3       = Y5_GPIO_NUM;
  config.pin_d4       = Y6_GPIO_NUM;
  config.pin_d5       = Y7_GPIO_NUM;
  config.pin_d6       = Y8_GPIO_NUM;
  config.pin_d7       = Y9_GPIO_NUM;
  config.pin_xclk     = XCLK_GPIO_NUM;
  config.pin_pclk     = PCLK_GPIO_NUM;
  config.pin_vsync    = VSYNC_GPIO_NUM;
  config.pin_href     = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn     = PWDN_GPIO_NUM;
  config.pin_reset    = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565;

  if(psramFound()) {
    config.frame_size = FRAMESIZE_QVGA;
    config.fb_count = 2;
    config.fb_location = CAMERA_FB_IN_PSRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.fb_count = 1;
    config.fb_location = CAMERA_FB_IN_DRAM;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  }

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed: 0x%x", err);
    return;
  }

  // Start Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");

  WiFiAddr = WiFi.localIP().toString();

  // Start camera streaming server (defined in app_httpd.cpp)
  startCameraServer();

  // Add our handlers for UI and fire
  server.on("/", handleRoot);
  server.on("/fire", handleFireMotor);
  server.begin();

  Serial.println("Web server started!");
  Serial.print("Visit: http://");
  Serial.println(WiFi.localIP());
}

void loop() {
  server.handleClient();
}
