/*
 * Transmitter Code for FPV Head Tracker Transmitter Unit 
 * Created by: <-- LOKESH SANJAY SAWANKAR --> 
 * Project Description: A DIY FPV Head Tracker device using NRF24L01 transceiver, MPU6050 sensor, servos, and Arduino Pro Mini.
 * For mounting to FPV Headset and controlling the FPV Head-tracking Camera Pan and Tilt unit.
 */

#include <Wire.h>
#include "MPU6050_6Axis_MotionApps20.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/* --- NRF Setup --- */
RF24 radio(9, 10);  // CE, CSN
const byte address[6] = "00001";
struct Data_Package {
  float yaw;
  float pitch;
};
Data_Package data;

/* --- MPU6050 Setup --- */
MPU6050 mpu;
#define INTERRUPT_PIN 2
bool blinkState;
bool dmpReady = false;
uint8_t mpuIntStatus;
uint8_t devStatus;
uint16_t packetSize;
uint8_t fifoBuffer[64];
Quaternion q;
VectorFloat gravity;
float ypr[3];
volatile bool mpuInterrupt = false;
void dmpDataReady() {
  mpuInterrupt = true;
}

void resetData()
{
  data.yaw = 0;
  data.pitch = 0;
}

/* --- Setup --- */
void setup() {
  Serial.begin(115200);

  // MPU6050 Initialization
  Wire.begin();
  Wire.setClock(400000);
  Serial.println(F("Initializing MPU6050..."));
  mpu.initialize();
  pinMode(INTERRUPT_PIN, INPUT);

  if (!mpu.testConnection()) {
    Serial.println(F("MPU6050 connection failed"));
    while (true);
  }
  Serial.println(F("MPU6050 connection successful"));

  Serial.println(F("Initializing DMP..."));
  devStatus = mpu.dmpInitialize();

  // Supply your gyro offsets here
  mpu.setXGyroOffset(0);
  mpu.setYGyroOffset(0);
  mpu.setZGyroOffset(0);
  mpu.setXAccelOffset(0);
  mpu.setYAccelOffset(0);
  mpu.setZAccelOffset(0);

  if (devStatus == 0) {
    mpu.CalibrateAccel(6);
    mpu.CalibrateGyro(6);
    Serial.println(F("Enabling DMP..."));
    mpu.setDMPEnabled(true);

    attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), dmpDataReady, RISING);
    mpuIntStatus = mpu.getIntStatus();
    Serial.println(F("DMP ready!"));
    dmpReady = true;
    packetSize = mpu.dmpGetFIFOPacketSize();
  } else {
    Serial.print(F("DMP Initialization failed (code "));
    Serial.print(devStatus);
    Serial.println(F(")"));
  }

  // NRF24L01 Initialization
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_LOW);
  resetData();
}

/* --- Loop --- */
void loop() {
  if (!dmpReady) return;

  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

    data.yaw = ypr[0] * 180 / M_PI;
    data.pitch = ypr[2] * 180 / M_PI;

    Serial.print("Yaw: ");
    Serial.print(data.yaw);
    Serial.print(" Pitch: ");
    Serial.println(data.pitch);

    // Transmit the yaw and pitch data via NRF24L01
    radio.write(&data, sizeof(Data_Package));
  }
  if (radio.write(&data, sizeof(Data_Package))) {
    Serial.println("Data sent successfully");
  } else {
    Serial.println("Failed to send data");
  }
}
