#include <AccelStepper.h>

// Distances are in mm

#define triggerPin 14
#define bottomEchoPin 15
#define verticalEchoPin 16
#define precision .3
#define ultrasonicRepetitions 10
#define maxAttempts 20

// Swap first and last pins
AccelStepper bottomStepper(AccelStepper::FULL4WIRE, 5, 3, 4, 2);
AccelStepper verticalStepper(AccelStepper::FULL4WIRE, 9, 7, 8, 6);

bool bottomStepping = false;
bool verticalStepping = false;

double bottomTargetDistance;
double bottomClosestDistance;
double bottomFarthestDistance;

double verticalTargetDistance;
double verticalClosestDistance;
double verticalFarthestDistance;

int bottomMovementAttempts = 0;
int verticalMovementAttempts = 0;

bool movingBySteps = false;
bool testingStepsPerMM = false;
double stepsPerMM;
int steppersDone = 2;

int mmToSteps(double mm) { return round(mm * stepsPerMM); };

bool bottomCloseEnough() {
  return abs(getBottomDistance() - bottomClosestDistance -
             bottomTargetDistance) < precision;
};

bool verticalCloseEnough() {
  return abs(getVerticalDistance() - verticalClosestDistance -
             verticalTargetDistance) < precision;
};

double getBottomDistance() {
  double sum = 0;
  for (int i = 0; i < ultrasonicRepetitions; i++) {
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    sum += pulseIn(bottomEchoPin, HIGH) * 0.343 / 2;
  }
  return sum / ultrasonicRepetitions;
}

double getVerticalDistance() {
  double sum = 0;
  for (int i = 0; i < ultrasonicRepetitions; i++) {
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    sum += pulseIn(verticalEchoPin, HIGH) * 0.343 / 2;
  }
  return sum / ultrasonicRepetitions;
}

void stepBottom(int steps) {
  bottomStepper.move(steps);
  bottomStepper.setSpeed(300);
  bottomStepping = true;
}

void stepVertical(int steps) {
  verticalStepper.move(steps);
  verticalStepper.setSpeed(300);
  verticalStepping = true;
}

void bottomAttemptStepToTarget() {
  stepBottom(mmToSteps(getBottomDistance() - bottomClosestDistance -
                       bottomTargetDistance));
}

void verticalAttemptStepToTarget() {
  stepVertical(mmToSteps(getVerticalDistance() - verticalClosestDistance -
                         verticalTargetDistance));
}

void setup() {
  pinMode(triggerPin, OUTPUT);
  pinMode(bottomEchoPin, INPUT);
  pinMode(verticalEchoPin, INPUT);
  Serial.begin(115200);
  bottomStepper.setMaxSpeed(300);
  verticalStepper.setMaxSpeed(300);
}

void loop() {
  if (Serial.available() > 0) {
    String raw = Serial.readStringUntil('\n');
    String key = raw.substring(0, raw.indexOf(':'));
    String data = raw.substring(raw.indexOf(':') + 1);

    if (key == "step") {
      double bottomSteps = data.substring(0, data.indexOf(',')).toDouble();
      double verticalSteps = data.substring(data.indexOf(',') + 1).toDouble();
      movingBySteps = true;
      stepBottom(bottomSteps);
      stepVertical(verticalSteps);
    } else if (key == "move") {
      double bottomDistance = data.substring(0, data.indexOf(',')).toDouble();
      double verticalDistance =
          data.substring(data.indexOf(',') + 1).toDouble();
      if (bottomDistance > bottomFarthestDistance - bottomClosestDistance ||
          verticalDistance >
              verticalFarthestDistance - verticalClosestDistance) {
        Serial.println("error:Target position is too far away");
      } else {
        bottomTargetDistance = bottomDistance;
        bottomStepping = true;
        verticalTargetDistance = verticalDistance;
        verticalStepping = true;
      }
    } else if (key == "setClosest") {
      if (data == "bottom") {
        bottomClosestDistance = getBottomDistance();
        testingStepsPerMM = true;
        stepBottom(-2000);
      } else if (data == "vertical") {
        verticalClosestDistance = getVerticalDistance();
      }
    } else if (key == "setFarthest") {
      if (data == "bottom") {
        bottomFarthestDistance = getBottomDistance();
      } else if (data == "vertical") {
        verticalFarthestDistance = getVerticalDistance();
        Serial.println("done");
      }
    }
  }

  if (bottomStepping && bottomStepper.distanceToGo() == 0) {
    bottomStepping = false;
    if (movingBySteps) {
      steppersDone++;
    } else if (testingStepsPerMM) {
      testingStepsPerMM = false;
      stepsPerMM = 2000 / (getBottomDistance() - bottomClosestDistance);
      Serial.println("done");
    } else {
      if (bottomCloseEnough()) {
        bottomMovementAttempts = 0;
        steppersDone++;
      } else if (bottomMovementAttempts < maxAttempts) {
        bottomAttemptStepToTarget();
      } else {
        bottomMovementAttempts = 0;
        Serial.println("error:Could not reach target");
      }
    }
  }

  if (verticalStepping && verticalStepper.distanceToGo() == 0) {
    verticalStepping = false;
    if (movingBySteps) {
      steppersDone++;
    } else {
      if (verticalCloseEnough()) {
        verticalMovementAttempts = 0;
        steppersDone++;
      } else if (verticalMovementAttempts < maxAttempts) {
        verticalAttemptStepToTarget();
      } else {
        verticalMovementAttempts = 0;
        Serial.println("error:Could not reach target");
      }
    }
  }

  if (steppersDone == 2) {
    steppersDone = 0;
    movingBySteps = false;
    Serial.println('done');
  }

  bottomStepper.runSpeedToPosition();
  verticalStepper.runSpeedToPosition();
}
