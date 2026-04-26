#include <Servo.h>

// ================== SERVOS ==================
Servo nozzleServo;
Servo funnelServo;

// ================== MOTOR PINS ==================
const int ENA = 5;
const int IN1 = 8;
const int IN2 = 9;
const int ENB = 6;
const int IN3 = 10;
const int IN4 = 11;

// ================== SERVO PINS ==================
const int nozzleServoPin = 2;
const int funnelServoPin = 3;

// ================== POTHOLE ULTRASONIC ==================
const int trigPin = 4;
const int echoPin = 7;

// ================== OBSTACLE ULTRASONIC ==================
const int obTrigPin = 13;
const int obEchoPin = A0;

// ================== BUZZER ==================
const int buzzerPin = 12;

// ================== DEBUG FLAG ==================
const bool DEBUG_SERIAL = true;   // set false for clean fast movement

// ================== GROUND DISTANCE (HYBRID) ==================
int GROUND_DISTANCE_CM = 3;

const int ROLLING_WINDOW = 10;
long rollingBuffer[ROLLING_WINDOW];
int rollingIndex = 0;
int rollingCount = 0;
long rollingGroundCM = 3;

// ================== FIXED PARAMETERS ==================
const int MIN_POTHOLE_DEPTH_CM   = 2;
const int OBSTACLE_THRESHOLD_CM  = 15;
const int MIN_POTHOLE_ANGLES     = 5;   // must be consecutive in slow scan

// ---- FILL CONTROL ----
const int FILL_COMPLETE_TOLERANCE_CM = 1;
const unsigned long MIN_FILL_TIME_MS = 800;
const unsigned long MAX_FILL_TIME_MS = 9000;
const int SLOW_SCAN_DELAY            = 120;

// ================== SERVO SCAN ==================
int scanAngle = 75;
int scanDir   = 1;
const int SCAN_LEFT          = 50;
const int SCAN_RIGHT         = 130;
unsigned long lastScanTime   = 0;
const int FAST_SCAN_INTERVAL = 15;

// ================== STATES ==================
enum State { MOVE_FAST_SCAN, SLOW_FULL_SCAN, ALIGN, FILL };
State systemState = MOVE_FAST_SCAN;

// ================== VARIABLES ==================
int detectedAngle       = 90;
int bestAngle           = 90;
int fastScanConsecutive = 0;   // consecutive pothole reads in fast scan

// ================== MOTOR ==================
void moveForward() {
  analogWrite(ENA, 65); analogWrite(ENB, 65);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  digitalWrite(IN4, HIGH);
}
void moveBackward() {
  analogWrite(ENA, 70); analogWrite(ENB, 70);
  digitalWrite(IN1, LOW);  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}
void stopRobot() {
  analogWrite(ENA, 0); analogWrite(ENB, 0);
}

// ================== ULTRASONIC ==================
long readUltrasonic(int tPin, int ePin) {
  digitalWrite(tPin, LOW);
  delayMicroseconds(2);
  digitalWrite(tPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(tPin, LOW);
  long d = pulseIn(ePin, HIGH, 25000);
  if (d == 0) return 0;
  return d * 0.034 / 2;
}

// Fast version — 3 readings, used while moving
long filteredDistanceFast(int tPin, int ePin) {
  long sum = 0; int cnt = 0;
  for (int i = 0; i < 3; i++) {
    long d = readUltrasonic(tPin, ePin);
    if (d > 0 && d < 60) { sum += d; cnt++; }
    delay(2);
  }
  if (cnt == 0) return 0;
  return sum / cnt;
}

// Accurate version — 5 readings, used when stopped
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

// ================== STARTUP CALIBRATION ==================
void calibrateGroundDistance() {
  Serial.println("Calibrating ground distance...");
  Serial.println("Place robot on flat ground. Calibrating in 2 seconds...");

  for (int i = 0; i < 3; i++) {
    digitalWrite(buzzerPin, HIGH); delay(150);
    digitalWrite(buzzerPin, LOW);  delay(150);
  }
  delay(2000);

  long sum = 0; int validCount = 0;
  for (int i = 0; i < 15; i++) {
    long d = readUltrasonic(trigPin, echoPin);
    Serial.print("  Reading "); Serial.print(i + 1);
    Serial.print(": "); Serial.print(d); Serial.println(" cm");
    if (d > 0 && d < 60) { sum += d; validCount++; }
    delay(100);
  }

  if (validCount >= 5) {
    GROUND_DISTANCE_CM = (int)(sum / validCount);
    rollingGroundCM    = GROUND_DISTANCE_CM;
    for (int i = 0; i < ROLLING_WINDOW; i++) rollingBuffer[i] = GROUND_DISTANCE_CM;
    rollingCount = ROLLING_WINDOW;
    Serial.print("Startup ground distance: ");
    Serial.print(GROUND_DISTANCE_CM); Serial.println(" cm");
    digitalWrite(buzzerPin, HIGH); delay(500); digitalWrite(buzzerPin, LOW);
  } else {
    GROUND_DISTANCE_CM = 3;
    rollingGroundCM    = 3;
    Serial.println("WARNING: Calibration failed. Using default 3 cm.");
    for (int i = 0; i < 5; i++) {
      digitalWrite(buzzerPin, HIGH); delay(80);
      digitalWrite(buzzerPin, LOW);  delay(80);
    }
  }
}

// ================== ROLLING AVERAGE ==================
void updateRollingGround(long newReading) {
  if (newReading <= 0 || newReading >= 60) return;
  if (abs(newReading - rollingGroundCM) > 2) return;

  rollingBuffer[rollingIndex] = newReading;
  rollingIndex = (rollingIndex + 1) % ROLLING_WINDOW;
  if (rollingCount < ROLLING_WINDOW) rollingCount++;

  long sum = 0;
  for (int i = 0; i < rollingCount; i++) sum += rollingBuffer[i];
  rollingGroundCM = sum / rollingCount;
}

// ================== FAST SCAN ==================
void fastScan() {
  if (millis() - lastScanTime >= FAST_SCAN_INTERVAL) {
    lastScanTime = millis();
    scanAngle += scanDir * 10;
    if (scanAngle >= SCAN_RIGHT) { scanAngle = SCAN_RIGHT; scanDir = -1; }
    if (scanAngle <= SCAN_LEFT)  { scanAngle = SCAN_LEFT;  scanDir = 1; }
    nozzleServo.write(scanAngle);
  }
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(trigPin, OUTPUT);   pinMode(echoPin, INPUT);
  pinMode(obTrigPin, OUTPUT); pinMode(obEchoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  nozzleServo.attach(nozzleServoPin);
  funnelServo.attach(funnelServoPin);
  nozzleServo.write(90);
  funnelServo.write(80);
  stopRobot();

  calibrateGroundDistance();
  Serial.println("Robot starting.");
}

// ================== LOOP ==================
void loop() {
  switch (systemState) {

    // ===== FAST MOVE + FAST SCAN =====
    case MOVE_FAST_SCAN: {
      moveForward();
      fastScan();

      // obstacle — fast read
      long obs = filteredDistanceFast(obTrigPin, obEchoPin);
      if (obs > 0 && obs <= OBSTACLE_THRESHOLD_CM) {
        stopRobot(); moveBackward(); delay(500); stopRobot();
        fastScanConsecutive = 0;   // reset on obstacle
        break;
      }

      // pothole — fast read
      long d = filteredDistanceFast(trigPin, echoPin);
      updateRollingGround(d);
      int depth = (int)(d - rollingGroundCM);

      // consecutive streak tracking
      if (depth >= MIN_POTHOLE_DEPTH_CM) {
        fastScanConsecutive++;
      } else {
        fastScanConsecutive = 0;   // any normal read resets streak
      }

      if (DEBUG_SERIAL) {
        Serial.println("========================================");
        Serial.print("  Raw Distance      : "); Serial.print(d);                  Serial.println(" cm");
        Serial.print("  Rolling Ground    : "); Serial.print(rollingGroundCM);     Serial.println(" cm");
        Serial.print("  Depth             : "); Serial.print(depth);               Serial.println(" cm");
        Serial.print("  Scan Angle        : "); Serial.print(scanAngle);           Serial.println(" deg");
        Serial.print("  Consecutive reads : "); Serial.print(fastScanConsecutive); Serial.println("/3");
        if (depth >= MIN_POTHOLE_DEPTH_CM)
          Serial.println("  Status            : POTHOLE CANDIDATE");
        else
          Serial.println("  Status            : Normal ground");
        Serial.println("========================================");
      }

      // only trigger slow scan after 3 consecutive pothole reads
      // 3 consecutive readings IS the confirmation — no extra step needed
      if (fastScanConsecutive >= 3) {
        fastScanConsecutive = 0;
        stopRobot();
        if (DEBUG_SERIAL) Serial.println(">>> 3 consecutive reads confirmed — starting slow scan.");
        detectedAngle = scanAngle;
        systemState   = SLOW_FULL_SCAN;
      }
      break;
    }

    // ===== SLOW FULL SCAN (CONSECUTIVE STREAK CHECK) =====
    case SLOW_FULL_SCAN: {
      int consecutiveNow  = 0;
      int bestStreakCount = 0;
      int bestStreakSum   = 0;
      int tempStreakSum   = 0;

      if (DEBUG_SERIAL) Serial.println("\n--- SLOW SCAN START ---");

      for (int angle = SCAN_LEFT; angle <= SCAN_RIGHT; angle += 2) {
        nozzleServo.write(angle);
        delay(SLOW_SCAN_DELAY);

        long d     = filteredDistanceCM(trigPin, echoPin);
        int  depth = (int)(d - rollingGroundCM);

        if (DEBUG_SERIAL) {
          Serial.print("  Angle ");
          if (angle < 100) Serial.print(" ");
          Serial.print(angle); Serial.print(" deg | ");
          Serial.print("Raw: "); Serial.print(d); Serial.print(" cm | ");
          Serial.print("Depth: "); Serial.print(depth); Serial.print(" cm | ");
        }

        if (depth >= MIN_POTHOLE_DEPTH_CM) {
          consecutiveNow++;
          tempStreakSum += angle;
          if (DEBUG_SERIAL) {
            Serial.print("<-- POTHOLE (streak: ");
            Serial.print(consecutiveNow); Serial.println(")");
          }
          if (consecutiveNow > bestStreakCount) {
            bestStreakCount = consecutiveNow;
            bestStreakSum   = tempStreakSum;
          }
        } else {
          consecutiveNow = 0;
          tempStreakSum  = 0;
          if (DEBUG_SERIAL) Serial.println("ground");
        }
      }

      if (DEBUG_SERIAL) {
        Serial.println("--- SLOW SCAN END ---");
        Serial.print("  Longest consecutive streak : "); Serial.println(bestStreakCount);
        Serial.print("  Required minimum           : "); Serial.println(MIN_POTHOLE_ANGLES);
      }

      if (bestStreakCount < MIN_POTHOLE_ANGLES) {
        if (DEBUG_SERIAL) Serial.println("  >> REJECTED — no continuous cluster. Resuming.\n");
        nozzleServo.write(90);
        systemState = MOVE_FAST_SCAN;
        break;
      }

      bestAngle = bestStreakSum / bestStreakCount;
      if (DEBUG_SERIAL) {
        Serial.print("  Best fill angle : "); Serial.print(bestAngle); Serial.println(" deg");
      }
      systemState = ALIGN;
      break;
    }

    // ===== ALIGN =====
    case ALIGN:
      if (DEBUG_SERIAL) Serial.println("\n>>> ALIGNING nozzle to pothole center...");
      digitalWrite(buzzerPin, HIGH); delay(200);
      digitalWrite(buzzerPin, LOW);
      nozzleServo.write(bestAngle);
      delay(800);
      if (DEBUG_SERIAL) Serial.println(">>> Aligned. Starting fill.");
      systemState = FILL;
      break;

    // ===== SMART FILL =====
    case FILL: {
      if (DEBUG_SERIAL) Serial.println("\n>>> FILL STARTED");
      unsigned long startTime = millis();
      funnelServo.write(160);

      while (true) {
        long d     = filteredDistanceCM(trigPin, echoPin);
        int  depth = (int)(d - rollingGroundCM);
        unsigned long elapsed = millis() - startTime;

        if (DEBUG_SERIAL) {
          Serial.print("  Filling | Elapsed: "); Serial.print(elapsed);
          Serial.print(" ms | Depth: ");         Serial.print(depth);
          Serial.println(" cm");
        }

        if (elapsed > MIN_FILL_TIME_MS && depth <= FILL_COMPLETE_TOLERANCE_CM) {
          if (DEBUG_SERIAL) Serial.println(">>> FILL COMPLETE — hole level reached.");
          break;
        }
        if (elapsed > MAX_FILL_TIME_MS) {
          if (DEBUG_SERIAL) Serial.println(">>> FILL TIMEOUT — max time reached.");
          break;
        }
        delay(100);
      }

      funnelServo.write(80);
      nozzleServo.write(90);
      if (DEBUG_SERIAL) Serial.println(">>> Funnel closed. Resuming patrol.\n");
      systemState = MOVE_FAST_SCAN;
      break;
    }
  }
}
