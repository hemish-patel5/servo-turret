#include <Arduino.h>
#include <ESP32Servo.h>

Servo horizontalServo;
Servo verticalServo;

constexpr int HORIZONTAL_SERVO_PIN = 18;
constexpr int VERTICAL_SERVO_PIN = 19;
constexpr int HORIZONTAL_MIN = 10;
constexpr int HORIZONTAL_MAX = 170;
constexpr int VERTICAL_MIN = 10;
constexpr int VERTICAL_MAX = 120;

int horizontalAngle = 90;
int verticalAngle = 60;

void setAngles(int horizontal, int vertical)
{
  horizontalAngle = constrain(horizontal, HORIZONTAL_MIN, HORIZONTAL_MAX);
  verticalAngle = constrain(vertical, VERTICAL_MIN, VERTICAL_MAX);
  horizontalServo.write(horizontalAngle);
  verticalServo.write(verticalAngle);
  Serial.printf("OK,%d,%d\n", horizontalAngle, verticalAngle);
}

void setup()
{
  Serial.begin(115200);
  Serial.setTimeout(20);

  horizontalServo.setPeriodHertz(50);
  verticalServo.setPeriodHertz(50);
  horizontalServo.attach(HORIZONTAL_SERVO_PIN, 500, 2400);
  verticalServo.attach(VERTICAL_SERVO_PIN, 500, 2400);

  setAngles(horizontalAngle, verticalAngle);
  Serial.println("READY - send horizontal,vertical followed by newline");
}

void loop()
{
  if (!Serial.available())
    return;

  // Python sends one command such as: 90,60\n
  String command = Serial.readStringUntil('\n');
  command.trim();
  int comma = command.indexOf(',');
  if (comma <= 0)
  {
    Serial.println("ERR - expected horizontal,vertical");
    return;
  }

  String horizontalText = command.substring(0, comma);
  String verticalText = command.substring(comma + 1);
  for (unsigned int i = 0; i < horizontalText.length(); ++i)
    if (!isDigit(horizontalText[i]))
      return;
  for (unsigned int i = 0; i < verticalText.length(); ++i)
    if (!isDigit(verticalText[i]))
      return;

  setAngles(horizontalText.toInt(), verticalText.toInt());
}
