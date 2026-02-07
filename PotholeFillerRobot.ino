#include <Servo.h>

// pinnig of all components..
Servo nozzleServo;
Servo funnelServo;

// motor pins
const int ENA = 5;
const int IN1 = 8;
const int IN2 = 9;

const int ENB = 6;
const int IN3 = 10;
const int IN4 = 11;

// pins for both servo motors
const int nozzleServoPin = 2;
const int funnelServoPin = 3;

// pins for detection sensor
const int trigPin = 4;
const int echoPin = 7;

// pins for obstacle sensor
const int obTrigPin = 13;
const int obEchoPin = A0;

//  BUZZER pin
const int buzzerPin = 12;

//FIXED PARAMETERS (CM ONLY) 
const int GROUND_DISTANCE_CM = 6;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                   
const int MIN_POTHOLE_DEPTH_CM = 3;                                                                                              
const int OBSTACLE_THRESHOLD_CM = 15;

// FILL CONTROL
const int FILL_COMPLETE_TOLERANCE_CM = 1;
const unsigned long MIN_FILL_TIME_MS = 800;
const unsigned long MAX_FILL_TIME_MS = 6000;

const int SLOW_SCAN_DELAY = 120;

//  SERVO SCAN 
int scanAngle = 75;
int scanDir = 1;
const int SCAN_LEFT = 50;
const int SCAN_RIGHT = 105;
unsigned long lastScanTime = 0;
const int FAST_SCAN_INTERVAL = 15;

enum State { MOVE_FAST_SCAN, SLOW_FULL_SCAN, ALIGN, FILL };
State systemState = MOVE_FAST_SCAN;

int detectedAngle = 90;
int bestAngle = 90;

void moveForward() {
  analogWrite(ENA, 65);
  analogWrite(ENB, 65);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}

void moveBackward() {
  analogWrite(ENA, 70);
  analogWrite(ENB, 70);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void stopRobot() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

long readUltrasonic(int tPin, int ePin) {
  digitalWrite(tPin, LOW); delayMicroseconds(2);
  digitalWrite(tPin, HIGH); delayMicroseconds(10);
  digitalWrite(tPin, LOW);
  long d = pulseIn(ePin, HIGH, 25000);
  if (d == 0) return 0;
  return d * 0.034 / 2;
}

long filteredDistanceCM(int tPin, int ePin) {
  long sum = 0; int cnt = 0;
  for (int i = 0; i < 5; i++) {
    long d = readUltrasonic(tPin, ePin);
    if (d > 0 && d < 60) { sum += d; cnt++; }
    delay(3);
  }
  if (cnt == 0) return 0;
  return sum / cnt;
}


void fastScan() {
  if (millis() - lastScanTime >= FAST_SCAN_INTERVAL) {
    lastScanTime = millis();
    scanAngle += scanDir * 10;

    if (scanAngle >= SCAN_RIGHT) { scanAngle = SCAN_RIGHT; scanDir = -1; }
    if (scanAngle <= SCAN_LEFT)  { scanAngle = SCAN_LEFT;  scanDir = 1; }

    nozzleServo.write(scanAngle);
  }
}


void setup() {
  Serial.begin(9600);

  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(obTrigPin, OUTPUT);
  pinMode(obEchoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  nozzleServo.attach(nozzleServoPin);
  funnelServo.attach(funnelServoPin);

  nozzleServo.write(90);
  funnelServo.write(80);
  stopRobot();

  Serial.println("System started (center alignment + smart fill)");
}

void loop() {

  switch (systemState) {

    case MOVE_FAST_SCAN: {
      moveForward();
      fastScan();

 
      long obs = filteredDistanceCM(obTrigPin, obEchoPin);
      if (obs > 0 && obs <= OBSTACLE_THRESHOLD_CM) {
        stopRobot();
        moveBackward();
        delay(500);
        stopRobot();
        break;
      }
      
      long d = filteredDistanceCM(trigPin, echoPin);
      int depth = d - GROUND_DISTANCE_CM;

      if (depth >= MIN_POTHOLE_DEPTH_CM) {
        detectedAngle = scanAngle;
        stopRobot();
        systemState = SLOW_FULL_SCAN;
      }
      break;
    }

    case SLOW_FULL_SCAN: {
      int angleSum = 0;
      int angleCount = 0;

      for (int angle = SCAN_LEFT; angle <= SCAN_RIGHT; angle += 2) {
        nozzleServo.write(angle);
        delay(SLOW_SCAN_DELAY);

        long d = filteredDistanceCM(trigPin, echoPin);
        int depth = d - GROUND_DISTANCE_CM;

        Serial.print("Angle: ");
        Serial.print(angle);
        Serial.print(" Depth(cm): ");
        Serial.println(depth);

        if (depth >= MIN_POTHOLE_DEPTH_CM) {
          angleSum += angle;
          angleCount++;
        }
      }

      if (angleCount > 0)
        bestAngle = angleSum / angleCount;
      else
        bestAngle = 90;

      systemState = ALIGN;
      break;
    }

    case ALIGN:
      digitalWrite(buzzerPin, HIGH); delay(200);
      digitalWrite(buzzerPin, LOW);

      nozzleServo.write(bestAngle);
      delay(800);
      systemState = FILL;
      break;

    case FILL: {
      unsigned long startTime = millis();
      funnelServo.write(160);  

      while (true) {
        long d = filteredDistanceCM(trigPin, echoPin);
        int depth = d - GROUND_DISTANCE_CM;
        unsigned long elapsed = millis() - startTime;

        Serial.print("Filling... Depth: ");
        Serial.println(depth);

        if (elapsed > MIN_FILL_TIME_MS && depth <= FILL_COMPLETE_TOLERANCE_CM)
          break;

        if (elapsed > MAX_FILL_TIME_MS)
          break;

        delay(100);
      }

      funnelServo.write(80);   
      nozzleServo.write(90);
      systemState = MOVE_FAST_SCAN;
      break;
    }
  }
}
