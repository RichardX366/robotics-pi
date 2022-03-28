#include <AccelStepper.h>

// Distances are in mm

#define stepsPerRevolution 2048
#define echoPin 15
#define triggerPin 14
#define precision 1
#define ultrasonicRepetitions 5
#define maxAttempts 10

// Swap first and last pins
AccelStepper bottomStepper(AccelStepper::FULL4WIRE, 5, 3, 4, 2);

bool stepping = false;
bool movingBySteps = false;
bool testingStepsPerMM = false;

double targetDistance;
double closestDistance;
double farthestDistance;
double stepsPerMM;

int movementAttempts = 0;

int mmToSteps(double mm) { return round(mm * stepsPerMM); };

bool closeEnough() {
  return abs(getDistance() - closestDistance - targetDistance) < precision;
};

double getDistance() {
  double sum = 0;
  for (int i = 0; i < ultrasonicRepetitions; i++) {
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    sum += pulseIn(echoPin, HIGH) * 0.343 / 2;
  }
  return sum / ultrasonicRepetitions;
}

void step(int steps) {
  bottomStepper.move(steps);
  bottomStepper.setSpeed(300);
  stepping = true;
}

void attemptStepToTarget() {
  step(mmToSteps(getDistance() - closestDistance - targetDistance));
}

void setup() {
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);
  Serial.begin(115200);
  bottomStepper.setMaxSpeed(300);
}

void loop() {
  if (Serial.available() > 0) {
    String raw = Serial.readStringUntil('\n');
    String key = raw.substring(0, raw.indexOf(':'));
    String data = raw.substring(raw.indexOf(':') + 1);

    if (key == "step") {
      movingBySteps = true;
      step(data.toDouble());
    } else if (key == "move") {
      if (data.toDouble() > farthestDistance - closestDistance) {
        Serial.println("error:Target position is too far away");
      } else {
        targetDistance = data.toDouble();
        stepping = true;
      }
    } else if (key == "tune") {
      if (data == "setClosest") {
        closestDistance = getDistance();
        testingStepsPerMM = true;
        step(-2000);
      } else if (data == "setFarthest") {
        farthestDistance = getDistance();
        Serial.println("done");
      }
    }
  }
  if (stepping && bottomStepper.distanceToGo() == 0) {
    stepping = false;
    if (movingBySteps) {
      Serial.println("done");
      movingBySteps = false;
    } else if (testingStepsPerMM) {
      testingStepsPerMM = false;
      stepsPerMM = 2000 / (getDistance() - closestDistance);
      Serial.println("done");
    } else {
      if (closeEnough()) {
        movementAttempts = 0;
        Serial.println("done");
      } else if (movementAttempts < maxAttempts) {
        attemptStepToTarget();
      } else {
        movementAttempts = 0;
        Serial.println("error:Could not reach target");
      }
    }
  }
  bottomStepper.runSpeedToPosition();
}
