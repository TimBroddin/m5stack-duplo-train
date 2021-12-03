#include "Lpf2Hub.h"
#include <M5Stack.h>

#define Faces_Encoder_I2C_ADDR 0X5E

int encoder_increment; //positive: clockwise nagtive: anti-clockwise
int encoder_value = 0;
uint8_t direction; //0: clockwise 1: anti-clockwise
uint8_t last_button, cur_button;

void GetValue(void)
{
  int temp_encoder_increment;

  Wire.requestFrom(Faces_Encoder_I2C_ADDR, 3);
  if (Wire.available())
  {
    temp_encoder_increment = Wire.read();
    cur_button = Wire.read();
  }
  if (temp_encoder_increment > 127)
  { //anti-clockwise
    direction = 1;
    encoder_increment = 256 - temp_encoder_increment;
  }
  else
  {
    direction = 0;
    encoder_increment = temp_encoder_increment;
  }
}

void Led(int i, int r, int g, int b)
{
  Wire.beginTransmission(Faces_Encoder_I2C_ADDR);
  Wire.write(i);
  Wire.write(r);
  Wire.write(g);
  Wire.write(b);
  Wire.endTransmission();
}

void colorWoosh(int r, int g, int b, int del)
{
  for (int i = 0; i < 12; i++)
  {
    Led(i, r, g, b);
    delay(del);
  }
}

// create a hub instance
Lpf2Hub myHub;

byte motorPort = (byte)DuploTrainHubPort::MOTOR;

void colorSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
}

void speedometerSensorCallback(void *hub, byte portNumber, DeviceType deviceType, uint8_t *pData)
{
  Lpf2Hub *myHub = (Lpf2Hub *)hub;
}

void setup()
{

  myHub.init();

  M5.begin(); // M5STACK INITIALIZE
  M5.Power.begin();
  M5.Lcd.setBrightness(200); // BRIGHTNESS = MAX 255
  M5.Lcd.fillScreen(WHITE);
  M5.Lcd.setTextSize(25);
  Wire.begin();
  randomSeed(analogRead(0));
  colorWoosh(0, 255, 0, 10);
  colorWoosh(0, 0, 0, 10);

  dacWrite(25, 0);
}

// main loop
void loop()
{
  int i;
  M5.update();

  if (myHub.isConnecting())
  {
    myHub.connectHub();
    if (myHub.isConnected())
    {
      Serial.println("Connected to Duplo Hub");

      delay(200);
      // connect color sensor and activate it for updates
      myHub.activatePortDevice((byte)DuploTrainHubPort::SPEEDOMETER, speedometerSensorCallback);
      delay(200);
      // connect speed sensor and activate it for updates
      myHub.activatePortDevice((byte)DuploTrainHubPort::COLOR, colorSensorCallback);
      delay(200);
    }
    else
    {
      Serial.println("Failed to connect to Duplo Hub");
    }
  }
  else
  {
    GetValue();
    if (direction)
    {
      encoder_value -= encoder_increment * 10;
    }
    else
    {
      encoder_value += encoder_increment * 10;
    }
    if (encoder_value > 100)
      encoder_value = 100;
    if (encoder_value < -100)
      encoder_value = -100;

    M5.Lcd.setCursor(100, 40);
    M5.Lcd.printf("%3d", encoder_value);

    if (encoder_increment != 0)
    {
      Serial.println("Setting speed to ");
      Serial.println(encoder_value);
      myHub.setBasicMotorSpeed(motorPort, encoder_value);
      if (encoder_value == 0)
      {
        myHub.playSound((byte)DuploTrainBaseSound::BRAKE);
      }
    }
    if (last_button != cur_button)
    {

      last_button = cur_button;
    }
    if (cur_button)
    {
    }
    else
    {
      myHub.playSound((byte)DuploTrainBaseSound::HORN);
      colorWoosh(255, 0, 0, 10);
    }

    if (M5.BtnA.wasReleased() || M5.BtnA.pressedFor(1000, 200))
    {
      myHub.playSound((byte)DuploTrainBaseSound::WATER_REFILL);
      colorWoosh(0, 0, 255, 10);
    }
    else if (M5.BtnB.wasReleased() || M5.BtnB.pressedFor(1000, 200))
    {
      myHub.playSound((byte)DuploTrainBaseSound::STATION_DEPARTURE);
      colorWoosh(0, 255, 0, 10);
    }
    else if (M5.BtnC.wasReleased() || M5.BtnC.pressedFor(1000, 200))
    {
      myHub.playSound((byte)DuploTrainBaseSound::STEAM);

      colorWoosh(0, 255, 0, 10);
      int color = random(11);
      Serial.print("Color: ");
      Serial.println(COLOR_STRING[color]);
      myHub.setLedColor((Color)color);
    }
  }

} // End of loop
