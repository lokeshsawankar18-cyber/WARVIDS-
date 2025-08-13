/*
 * Receiver Code for Observation of proper output through transmiter  
 * Created by: <-- LOKESH SANJAY SAWANKAR --> 
 * Project Description: 10 Channel reciver observation on serial monitor. 
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <Servo.h>

#define SIGNAL_TIMEOUT 500

const uint64_t pipeIn = 0xF9E8F0F0E1LL;
RF24 radio(8, 9);
unsigned long lastRecvTime = 0;

Servo servo1; // controlled by switch5
Servo servo2; // controlled by switch6

struct PacketData
{
  byte lxAxisValue;
  byte lyAxisValue;
  byte rxAxisValue;
  byte ryAxisValue;
  byte lPotValue;  
  byte rPotValue;    
  byte switch1Value;
  byte switch2Value;
  byte switch3Value;
  byte switch4Value;
  byte switch5Value;
  byte switch6Value;
};
PacketData receiverData;

void setup()
{
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openReadingPipe(1, pipeIn);
  radio.startListening();

  Serial.begin(115200);

  servo1.attach(5); // attach servo1 to pin 5
  servo2.attach(6); // attach servo2 to pin 6
}

void loop()
{
  if (radio.isChipConnected() && radio.available())
  {
    radio.read(&receiverData, sizeof(PacketData));
    lastRecvTime = millis();

    // Update Servos based on Switch5 and Switch6
    if (receiverData.switch5Value)
      servo1.write(180);
    else
      servo1.write(0);

    if (receiverData.switch6Value)
      servo2.write(90);
    else
      servo2.write(0);

    // Print all data to Serial Monitor
    char inputValuesString[130];
    sprintf(inputValuesString,
            "%3d,%3d,%3d,%3d,%3d,%3d,%d,%d,%d,%d,%d,%d",
            receiverData.lxAxisValue,
            receiverData.lyAxisValue,
            receiverData.rxAxisValue,
            receiverData.ryAxisValue,
            receiverData.lPotValue,
            receiverData.rPotValue,
            receiverData.switch1Value,
            receiverData.switch2Value,
            receiverData.switch3Value,
            receiverData.switch4Value,
            receiverData.switch5Value,
            receiverData.switch6Value);
    Serial.println(inputValuesString);
  }
  else
  {
    unsigned long now = millis();
    if (now - lastRecvTime > SIGNAL_TIMEOUT)
    {
      Serial.println("No Signal");
    }
  }
}
