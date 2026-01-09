#include <ESP32Servo.h>

//constants
const int SERVO_PIN = 32; // Servo control pin
const int LDR_TOP    = 36; // LDR1 
const int LDR_LEFT   = 39; // LDR2
const int LDR_RIGHT  = 34; // LDR3
const int LDR_BOTTOM = 35; // LDR4
const float VREF     = 3.3; // ADC ref voltage
const int ADC_MAX    = 4095; // 12-bit ADC
const float DEADBAND = 0.15; // Min voltage diff to move
const float SCALE    = 150.0; // Scaling for servo response
const int SERVO_MIN  = 20; // Servo angle limits (min)
const int SERVO_MAX  = 160; // Servo angle limits (max)
Servo azServo;
int servoAngle = 90;

// Baselines
float baselineLeft  = 0;
float baselineRight = 0;
float centerBias    = 0;

void setup() {
  Serial.begin(115200);
  azServo.attach(SERVO_PIN);
  azServo.write(servoAngle);
  delay(2000);
 // baseline voltages
  baselineLeft  = analogRead(LDR_LEFT)  * VREF / ADC_MAX;
  baselineRight = analogRead(LDR_RIGHT) * VREF / ADC_MAX;
// center bias
  centerBias = baselineLeft - baselineRight;
  Serial.println("Baseline:");
  Serial.print("Left: "); Serial.println(baselineLeft, 3);
  Serial.print("Right: "); Serial.println(baselineRight, 3);
  Serial.print("Bias: "); Serial.println(centerBias, 3);
}
void loop() {
  // Read LDRs
  float leftV  = analogRead(LDR_LEFT)  * VREF / ADC_MAX;
  float rightV = analogRead(LDR_RIGHT) * VREF / ADC_MAX;
  // Calculate error (positive -> move toward left light)
  float error = (leftV - rightV) - centerBias;
  // Servo control with deadband and scaling
  if (abs(error) <= DEADBAND) {
    // Centered: snap to 90°
    servoAngle = 90;
  } else {
    servoAngle = 90 + int(error * SCALE);
  }
// Constrain servo angle
  servoAngle = constrain(servoAngle, SERVO_MIN, SERVO_MAX);
// Write to servo
  azServo.write(servoAngle);
  // Debugging output
  Serial.print("L: "); Serial.print(leftV, 2);
  Serial.print(" R: "); Serial.print(rightV, 2);
  Serial.print(" Diff: "); Serial.print(error, 3);
  Serial.print(" Servo: "); Serial.println(servoAngle);
  delay(15); // update frequency around 66 Hz
}
