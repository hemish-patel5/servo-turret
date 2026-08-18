#include <Arduino.h>
#include <ESP32Servo.h>

Servo horizontalServo;
Servo verticalServo;

constexpr int HORIZONTAL_SERVO_PIN = 18;
constexpr int VERTICAL_SERVO_PIN = 19;
constexpr int STEP_SIZE = 10;

int horizontalAngle = 90;
int verticalAngle = 90;

void moveServo(Servo &servo, int &angle, int change)
{
  angle = constrain(angle + change, 0, 180);
  servo.write(angle);

  Serial.print("Horizontal: ");
  Serial.print(horizontalAngle);
  Serial.print(" degrees, Vertical: ");
  Serial.print(verticalAngle);
  Serial.println(" degrees");
}

void setup()
{
  Serial.begin(115200);

  horizontalServo.setPeriodHertz(50);
  verticalServo.setPeriodHertz(50);
  horizontalServo.attach(HORIZONTAL_SERVO_PIN, 500, 2400);
  verticalServo.attach(VERTICAL_SERVO_PIN, 500, 2400);

  horizontalServo.write(horizontalAngle);
  verticalServo.write(verticalAngle);

  Serial.println("Arrow-key servo control ready.");
  Serial.println("Left/Right: GPIO 18 | Up/Down: GPIO 19");
}

void loop()
{
  if (Serial.available() < 3)
  {
    return;
  }

  // Arrow keys are sent by most terminals as: ESC [ A/B/C/D.
  if (Serial.read() != 27 || Serial.read() != '[')
  {
    return;
  }

  switch (Serial.read())
  {
  case 'A': // Up
    moveServo(verticalServo, verticalAngle, STEP_SIZE);
    break;
  case 'B': // Down
    moveServo(verticalServo, verticalAngle, -STEP_SIZE);
    break;
  case 'C': // Right
    moveServo(horizontalServo, horizontalAngle, -STEP_SIZE);
    break;
  case 'D': // Left
    moveServo(horizontalServo, horizontalAngle, STEP_SIZE);
    break;
  default:
    break;
  }
}
