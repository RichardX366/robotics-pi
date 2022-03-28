#include <AccelStepper.h>

const int stepsPerRevolution = 2048;

// Swap first and last pins
AccelStepper stepper(AccelStepper::FULL4WIRE, 5, 3, 4, 2);
bool stepping = false;

void setup() {
  Serial.begin(115200);
  stepper.setMaxSpeed(300);
}

void loop() {
  if (Serial.available() > 0) {
    String raw = Serial.readStringUntil('\n');
    String key = raw.substring(0, raw.indexOf(':'));
    String data = raw.substring(raw.indexOf(':') + 1);

    if (key == "step") {
      stepper.move(data.toDouble());
      stepper.setSpeed(300);
      stepping = true;
    } else if (key == "pins") {
      stepper = AccelStepper(AccelStepper::FULL4WIRE, data[0] - '0',
                             data[1] - '0', data[2] - '0', data[3] - '0');
      stepper.setMaxSpeed(300);
      Serial.println("done");
    }
  }
  if (stepping && stepper.distanceToGo() == 0) {
    Serial.println("done");
    stepping = false;
  }
  stepper.runSpeedToPosition();
}
