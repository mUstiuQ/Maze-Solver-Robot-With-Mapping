#include <SPI.h>
#include <RF24.h>
#include <Servo.h>

RF24 radio(7, 11); 
const byte address[6] = "00001";

// Pini PWM
#define IN1 5
#define IN2 4
#define IN3 3
#define IN4 2

#define ENA 9  // PWM pentru motoare stanga
#define ENB 10 // PWM pentru motoare dreapta

#define SERVO_PIN A0
#define TRIG A3
#define ECHO A4
#define BUZZER 6

Servo scanner;

struct Packet {
  int accel;
  int brake;
  int steer;
  bool dance;
};

Packet data;

// Variabile pentru cronometrare Bip-Bip (fara delay)
unsigned long lastBuzzerTime = 0;
//int buzzerState = LOW;
const long buzzerInterval = 150; // Cat de des face bip (milisecunde)
bool buzzerState = false;


long getDistance() {
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  long duration = pulseIn(ECHO, HIGH, 30000);
  if (duration == 0) return 999; 
  return duration * 0.034 / 2;
}

// Logica Motoare (Cablajul tau: Stanga IN2=Fata / Dreapta IN3=Fata)
void motors(int left, int right) {
  // Determinăm direcția pentru stânga
  if (left > 0) {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
  } else if (left < 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
  } else {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
  }

  // Determinăm direcția pentru dreapta
  if (right > 0) {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  } else if (right < 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  } else {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }

  // APLICĂM VITEZA REALA (PWM)
  analogWrite(ENA, abs(left));
  analogWrite(ENB, abs(right));
}

void stopAll() {
  motors(0, 0);
}

bool isDancing = false;
unsigned long danceStart = 0;
int danceStep = 0;

void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);

  scanner.attach(SERVO_PIN);
  scanner.write(90);

  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);

  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_LOW);
  radio.startListening();
}


// In loop(), adaugă o variabilă pentru Failsafe
unsigned long lastRecvTime = 0;
bool scanDone = false;  // am scanat deja, astept sa se elibereze

void loop() {
  if (radio.available()) {
    radio.read(&data, sizeof(data));
    lastRecvTime = millis();
  }

  // FAILSAFE
  if (millis() - lastRecvTime > 1000) {
    stopAll();
    return;
  }

  unsigned long currentMillis = millis();
/*
  // --- SCANNING MODE ---
  if (isScanning) {
    if (currentMillis - scanMoveTime < SCAN_SETTLE) return;

    int d = getDistance();

    if (scanStep == 0) {
      distLeft = d;
      Serial.print("Scan L="); Serial.println(distLeft);
      scanStep = 1;
      scanner.write(150);  // look right
      scanMoveTime = currentMillis;
    }
    else if (scanStep == 1) {
      distRight = d;
      Serial.print("Scan R="); Serial.println(distRight);
      scanStep = 2;
      scanner.write(90);  // back to center
      scanMoveTime = currentMillis;
    }
    else {
      isScanning = false;
      scanDone = true;
      Serial.print("Done: L="); Serial.print(distLeft);
      Serial.print(" R="); Serial.println(distRight);
    }

    Serial.print("SCANNING step="); Serial.print(scanStep);
    Serial.print(" d="); Serial.println(d);
    return;
  }
*/

    // DANCE MODE
  static bool lastDance = false;
  if (data.dance && !lastDance && !isDancing) {
    isDancing = true;
    danceStart = currentMillis;
    Serial.println("DANCE START!");
  }
  lastDance = data.dance;

 if (isDancing) {
    unsigned long t = currentMillis - danceStart;
    
    // 360 spin
    if (t < 800)        motors(200, -200);
    else if (t < 1000)  motors(0, 0);
    
    // opt 1
    else if (t < 1800)  motors(100, 200);
    else if (t < 2600)  motors(200, 100);
    
    // opt 2
    else if (t < 2800)  motors(0, 0);
    else if (t < 3600)  motors(200, 100);
    else if (t < 4400)  motors(100, 200);
    
    // 360 final
    else if (t < 4600)  motors(0, 0);
    else if (t < 5400)  motors(-200, 200);
    
    // beep beep
    else if (t < 5600) { motors(0, 0); digitalWrite(BUZZER, HIGH); }
    else if (t < 5800) { motors(0, 0); digitalWrite(BUZZER, LOW); }
    else if (t < 6000) { motors(0, 0); digitalWrite(BUZZER, HIGH); }
    else if (t < 6200) { motors(0, 0); digitalWrite(BUZZER, LOW); }
    else {
      stopAll();
      digitalWrite(BUZZER, LOW);
      isDancing = false;
    }
    return;
  }
  


  // --- DISTANCE ---
  int dist = getDistance();

   // BUZZER PROGRESIV
  if (dist > 0 && dist < 20) {
    unsigned long interval = map(constrain(dist, 5, 20), 5, 20, 50, 500);
    if (currentMillis - lastBuzzerTime >= interval) {
      lastBuzzerTime = currentMillis;
      buzzerState = !buzzerState;
      digitalWrite(BUZZER, buzzerState ? HIGH : LOW);
    }
  } else {
    digitalWrite(BUZZER, LOW);
    buzzerState = false;
  }

  // MISCARE LIBERA
  int forward = data.accel - data.brake;
  int steerAmount = data.steer;
  int leftMotor  = forward - (steerAmount * 1.5);
  int rightMotor = forward + (steerAmount * 1.5);

  // Pivot turning
  if (abs(forward) < 10 && abs(data.steer) > 15) {
    int pivotSpeed = map(abs(data.steer), 15, 135, 70, 180);
    if (data.steer > 0) {
      leftMotor = pivotSpeed;
      rightMotor = -pivotSpeed;
    } else {
      leftMotor = -pivotSpeed;
      rightMotor = pivotSpeed;
    }
  }

  motors(constrain(leftMotor, -255, 255), constrain(rightMotor, -255, 255));

  delay(10); 
}