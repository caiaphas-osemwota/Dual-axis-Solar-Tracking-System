#include <ESP32Servo.h>

/*
  DualAxisSolarTracker

  Active firmware for the ESP32 dual-axis solar-tracking prototype.

  Hardware:
  - ESP32
  - Four LDRs: top, bottom, left, and right
  - Two servos: azimuth and elevation

  Dependency:
  - ESP32Servo

  The sketches in archive/ are earlier experiments and are not the active
  tracker firmware.
*/

//constants
const int AZSERVO_PIN  = 32; // Azimuth Servo control pin
const int ELVSERVO_PIN = 33; // Elevation Servo control pin
const int LDR_TOP    = 36; // LDR1 
const int LDR_LEFT   = 39; // LDR2
const int LDR_RIGHT  = 34; // LDR3
const int LDR_BOTTOM = 35; // LDR4
const float VREF     = 3.3; // ADC ref voltage
const int ADC_MAX    = 4095; // 12-bit ADC
const float DEADBAND = 0.15; // Min voltage diff to move
const int AZ_MIN  = 20; // Servo angle limits (min)
const int AZ_MAX  = 160; // Servo angle limits (max)
const int ELV_MIN = 40;
const int ELV_MAX = 120;
const int baseline_samples = 20; // # of taken samples to average
const int baseline_sampledelay = 10; // ms between samples

Servo azServo;
Servo elvServo;
int AzservoAngle = 90;
int ElvservoAngle = 90;

// Baselines
float baselineLeft  = 0;
float baselineRight = 0;
float baselineTop = 0;
float baselineBottom = 0;
float HorizontalBias = 0;
float VerticalBias = 0;

//EWA Filter
const float ALPHA = 0.2;
float filtLeft = 0, filtRight = 0, filtTop = 0, filtBottom = 0;
bool filtInit = false;

// PID Gains
float Kp_az = 50.0, Ki_az = 0.0, Kd_az = 0.0;
float Kp_el = 50.0, Ki_el = 0.0, Kd_el = 0.0;

// PID state
float azIntegral = 0, elvIntegral = 0;
float azPrevError = 0, elvPrevError = 0;

const float INTEGRAL_LIMIT = 50.0;
unsigned long lastLoopTime = 0;

float pidStep(float error, float &integral, float &prevError, float Kp, float Ki, float Kd, float dt) {
  if (abs(error) <= DEADBAND) {
    prevError = error;
    return NAN;
  }
  integral += error * dt;
  integral = constrain(integral, -INTEGRAL_LIMIT, INTEGRAL_LIMIT);
  float derivative = (error - prevError) / dt;
  prevError = error;
  return Kp * error + Ki * integral + Kd * derivative;
}

float readAveragedLDR (int pin) {
  long sum = 0;
  for (int i = 0; i < baseline_samples; i++) {
    sum += analogRead(pin);
    delay(baseline_sampledelay);
  }
  float avgRaw = sum / (float) baseline_samples;
  return avgRaw * VREF / ADC_MAX;
}

void setup() {
  Serial.begin(115200);
  azServo.attach(AZSERVO_PIN);
  elvServo.attach(ELVSERVO_PIN);
  azServo.write(AzservoAngle);
  elvServo.write(ElvservoAngle);
  delay(2000);
 // baseline voltages
  baselineLeft = readAveragedLDR(LDR_LEFT);
  baselineRight = readAveragedLDR(LDR_RIGHT);
  baselineTop = readAveragedLDR(LDR_TOP);
  baselineBottom = readAveragedLDR(LDR_BOTTOM);
// center bias
  HorizontalBias = baselineLeft - baselineRight;
  VerticalBias = baselineTop - baselineBottom;
  
  lastLoopTime = millis();

  Serial.println("Baseline:");
  Serial.print("Left: "); Serial.println(baselineLeft, 3);
  Serial.print("Right: "); Serial.println(baselineRight, 3);
  Serial.print("Top: "); Serial.println(baselineTop, 3);
  Serial.print("Bottom: "); Serial.println(baselineBottom, 3);
  Serial.print("Horizontal Bias: "); Serial.println(HorizontalBias, 3);
  Serial.print("Vertical Bias: "); Serial.println(VerticalBias, 3);
}

void loop() {
  unsigned long now = millis();
  float dt = (now - lastLoopTime) / 1000.0; // actual elapsed seconds
  if (dt <= 0.001) dt = 0.015;
  lastLoopTime = now;
  // Read LDRs
  float leftV_raw = analogRead(LDR_LEFT) * VREF / ADC_MAX;
  float rightV_raw = analogRead(LDR_RIGHT) * VREF / ADC_MAX;
  float topV_raw = analogRead(LDR_TOP) * VREF / ADC_MAX;
  float bottomV_raw = analogRead(LDR_BOTTOM) * VREF / ADC_MAX;
  
  if (!filtInit) {
  filtLeft = leftV_raw; filtRight = rightV_raw;
  filtTop = topV_raw; filtBottom = bottomV_raw;
  filtInit = true;
  }
  filtLeft += ALPHA * (leftV_raw - filtLeft);
  filtRight += ALPHA * (rightV_raw - filtRight);
  filtTop += ALPHA * (topV_raw - filtTop);
  filtBottom += ALPHA * (bottomV_raw - filtBottom);

  // Calculate error
  float horizontalerror = (filtLeft - filtRight) - HorizontalBias;
  float verticalerror = (filtTop - filtBottom) - VerticalBias;
  // Servo control with deadband and scaling
  
  float azOutput = pidStep(horizontalerror, azIntegral, azPrevError, Kp_az, Ki_az, Kd_az, dt);
  if (!isnan(azOutput)) {
    AzservoAngle = constrain(90 + int(azOutput), AZ_MIN, AZ_MAX);
  }

  float elvOutput = pidStep(verticalerror, elvIntegral, elvPrevError, Kp_el, Ki_el, Kd_el, dt);
  if (!isnan(elvOutput)) {
    ElvservoAngle = constrain(90 + int(elvOutput), ELV_MIN, ELV_MAX);
  }

// Write to servo
  azServo.write(AzservoAngle);
  elvServo.write(ElvservoAngle);
  
  // Debugging output

  Serial.print(" Filtered L: "); Serial.print(filtLeft, 2);
  Serial.print(" Filtered R: "); Serial.print(filtRight, 2);
  Serial.print(" Filtered T: "); Serial.print(filtTop, 2);
  Serial.print(" Filtered B: "); Serial.print(filtBottom, 2);

  Serial.print(" Raw L: "); Serial.print(leftV_raw, 2);
  Serial.print(" Raw R: "); Serial.print(rightV_raw, 2);
  Serial.print(" Raw T: "); Serial.print(topV_raw, 2);
  Serial.print(" Raw B: "); Serial.print(bottomV_raw, 2);

  Serial.print(" Horizontal Diff: "); Serial.print(horizontalerror, 3);
  Serial.print(" Vertical Diff: "); Serial.print(verticalerror, 3);
  Serial.print(" Azimuth Servo: "); Serial.println(AzservoAngle);
  Serial.print(" Elevation Servo: "); Serial.println(ElvservoAngle);  

  delay(15); // update frequency around 66 Hz
}
