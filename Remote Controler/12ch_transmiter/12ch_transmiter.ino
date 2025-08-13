/*
 * 12 channel Transmitter Code 
 * Created by: <-- LOKESH SANJAY SAWANKAR --> 
 * Project Description: 10 Channel Transmiter for controling the WARVIDS Rover car and other components 
 */
 
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const uint64_t pipeOut = 0xF9E8F0F0E1LL;

RF24 radio(8, 9); // CE, CSN

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
PacketData data;

void setup()
{
  radio.begin();
  radio.setDataRate(RF24_250KBPS);
  radio.openWritingPipe(pipeOut);
  radio.stopListening();

  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(6, INPUT_PULLUP);  // New Switch5
  pinMode(7, INPUT_PULLUP);  // New Switch6
}

int mapAndAdjustJoystickDeadBandValues(int value, bool reverse)
{
  if (value >= 530)
    value = map(value, 530, 1023, 127, 254);
  else if (value <= 500)
    value = map(value, 500, 0, 127, 0);  
  else
    value = 127;

  return reverse ? 254 - value : value;
}

void loop()
{
  data.lxAxisValue    = mapAndAdjustJoystickDeadBandValues(analogRead(A0), false);
  data.lyAxisValue    = mapAndAdjustJoystickDeadBandValues(analogRead(A1), false);
  data.rxAxisValue    = mapAndAdjustJoystickDeadBandValues(analogRead(A2), false);
  data.ryAxisValue    = mapAndAdjustJoystickDeadBandValues(analogRead(A3), false);
  data.lPotValue      = map(analogRead(A4), 0, 1023, 0, 254);
  data.rPotValue      = map(analogRead(A5), 0, 1023, 0, 254);  
  data.switch1Value   = !digitalRead(2);
  data.switch2Value   = !digitalRead(3);
  data.switch3Value   = !digitalRead(4);
  data.switch4Value   = !digitalRead(5);
  data.switch5Value   = !digitalRead(6);
  data.switch6Value   = !digitalRead(7);

  radio.write(&data, sizeof(PacketData));
}
