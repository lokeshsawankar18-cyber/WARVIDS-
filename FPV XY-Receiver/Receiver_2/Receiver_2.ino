#include <SPI.h>
#include <RF24.h>
#include <Servo.h>

// NRF24L01
RF24 radio(9, 10); // CE, CSN pins for NRF24L01
const byte address[6] = "00001";

// Servos
Servo panServo;
Servo tiltServo;

// Data received
struct Data_Packet {
  float pitch;
  float yaw;
};

Data_Packet data;

int lastTiltAngle = 90; // Initialize tilt angle (neutral position)

// Smoothness factor for tilt movement (smaller = smoother)
int smoothFactor = 5; 

void setup() {
  Serial.begin(9600);

  // Initialize NRF24L01
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  // Attach servos
  panServo.attach(3);  // Pin connected to pan servo
  tiltServo.attach(5); // Pin connected to tilt servo
}

void loop() {
  // Check for data
  if (radio.available()) {
    radio.read(&data, sizeof(data));

    // Map and move servos based on pitch and yaw
    int panAngle = map(data.yaw, -90, 90, 180, 0);   // Adjust range as needed
    int targetTiltAngle = map(data.pitch, -90, 90, 180, 0); // Adjust range as needed

    // Smoothly move tilt servo
    if (abs(targetTiltAngle - lastTiltAngle) > smoothFactor) {
      if (targetTiltAngle > lastTiltAngle) {
        lastTiltAngle += smoothFactor;
      } else {
        lastTiltAngle -= smoothFactor;
      }
    } else {
      lastTiltAngle = targetTiltAngle; // If within smoothFactor, set directly
    }

    panServo.write(panAngle);
    tiltServo.write(lastTiltAngle);

    Serial.print("Pan: ");
    Serial.print(panAngle);
    Serial.print(" | Tilt: ");
    Serial.println(lastTiltAngle);
  }

  delay(20); // Adjust for smoother movement
}
