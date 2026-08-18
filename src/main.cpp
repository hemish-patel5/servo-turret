#include <Arduino.h>
#include <ESP32Servo.h>

Servo servo1;
Servo servo2;

const int SERVO1_PIN = 18;
const int SERVO2_PIN = 19;

void setup()
{
  servo1.attach(SERVO1_PIN);
  servo2.attach(SERVO2_PIN);

  // Start both servos at 0 degrees
  servo1.write(0);
  servo2.write(0);

  delay(1000);
}

void loop()
{

  // Move from 0 to 180 degrees
  for (int angle = 0; angle <= 180; angle++)
  {
    servo1.write(angle);
    servo2.write(angle);

    delay(15);
  }

  delay(500);

  // Move from 180 back to 0 degrees
  for (int angle = 180; angle >= 0; angle--)
  {
    servo1.write(angle);
    servo2.write(angle);

    delay(15);
  }

  delay(500);
}