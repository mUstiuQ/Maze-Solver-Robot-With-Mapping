#include <Wire.h>
#include <MPU6050_tockn.h>
#include <SPI.h>
#include <RF24.h>

RF24 radio(7, 8);  // CE=7, CSN=8
const byte address[6] = "00001";

MPU6050 mpu(Wire);

struct Packet {
  int accel;
  int brake;
  int steer;
  bool dance;
};

Packet data;

bool danceMode = false;
unsigned long lastShakeTime = 0;

// --- smoothing heavy ---
float ax_f = 0;
float ay_f = 0;

// FILTRU EXPONENTIAL (L-am lasat exact cum ai vrut)
float smooth(float prev, float now, float k) {
  return prev + (now - prev) * k;
}

void setup() {
  Serial.begin(115200);

  Wire.begin();
  mpu.begin();
  delay(2000);
  mpu.calcGyroOffsets(); // Tine mana nemiscata aici!

  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.openWritingPipe(address);
  radio.stopListening();

  Serial.println("Transmitter ULTRA HEAVY REPARAT!");
}

void loop() {
  mpu.update();

  // SHAKE DETECTION
  float accX = mpu.getAccX();
  float accY = mpu.getAccY();
  float accZ = mpu.getAccZ();
  float totalAcc = sqrt(accX*accX + accY*accY + accZ*accZ);
  
  if (totalAcc > 2.5 && millis() - lastShakeTime > 2000) {
    danceMode = !danceMode;
    lastShakeTime = millis();
    Serial.println(danceMode ? "DANCE ON!" : "DANCE OFF!");
  }

 float raw_X = mpu.getAngleX(); 
  float raw_Y = mpu.getAngleY(); 

  // --- FILTRARE ---
  ay_f = smooth(ay_f, raw_Y, 0.10); 
  ax_f = smooth(ax_f, raw_X, 0.10); 

  int deadzone = 22; 
  int maxAngle = 45;
  int minPWM = 85;  
  int maxPWM = 225; 
  int steerPower = 135;

  // ==========================================
  // 1. LOGICA FATA / SPATE (Viteză)
  // Am eliminat minusul de la fs_value pentru a inversa sensul
  // ==========================================
  float fs_value = ax_f; 

  if (fs_value >= deadzone) {
    data.accel = map(fs_value, deadzone, maxAngle, minPWM, maxPWM);
    data.brake = 0;
  }
  else if (fs_value <= -deadzone) {
    data.brake = map(abs(fs_value), deadzone, maxAngle, minPWM, maxPWM);
    data.accel = 0;
  }
  else {
    data.accel = 0;
    data.brake = 0;
  }

  // ==========================================
  // 2. LOGICA STEERING (Direcție)
  // Am eliminat minusul din map() pentru a inversa stânga cu dreapta
  // ==========================================
  if (abs(ay_f) < deadzone) {
    data.steer = 0;
  }
  else {
    // Acum ay_f pozitiv va fi Dreapta, negativ va fi Stânga
    data.steer = map(ay_f, -maxAngle, maxAngle, steerPower, -steerPower);
  }

  // LIMITĂRI DE SIGURANȚĂ
  data.accel = constrain(data.accel, 0, maxPWM);
  data.brake = constrain(data.brake, 0, maxPWM);
  data.steer = constrain(data.steer, -steerPower, steerPower);

  data.dance = danceMode;

  // TRIMITE PACHETUL
  radio.write(&data, sizeof(data));

  // DEBUG (Uită-te aici dacă ceva e încă inversat)
  Serial.print("Viteza: "); Serial.print(data.accel - data.brake);
  Serial.print(" | Directie: "); Serial.println(data.steer);


  delay(50); // Delay-ul tau original
}