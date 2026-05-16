#include <ESP32Servo.h>

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
const float SCALE    = 150.0; // Scaling for servo response
const int AZ_MIN  = 20; // Servo angle limits (min)
const int AZ_MAX  = 160; // Servo angle limits (max)
const int ELV_MIN = 40;
const int ELV_MAX = 120;
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

void setup() {
  Serial.begin(115200);
  azServo.attach(AZSERVO_PIN);
  elvServo.attach(ELVSERVO_PIN);
  azServo.write(AzservoAngle);
  elvServo.write(ElvservoAngle);
  delay(2000);
 // baseline voltages
  baselineLeft  = analogRead(LDR_LEFT)  * VREF / ADC_MAX;
  baselineRight = analogRead(LDR_RIGHT) * VREF / ADC_MAX;
  baselineTop = analogRead(LDR_TOP) * VREF / ADC_MAX;
  baselineBottom = analogRead(LDR_BOTTOM) * VREF / ADC_MAX;
// center bias
  HorizontalBias = baselineLeft - baselineRight;
  VerticalBias = baselineTop - baselineBottom;

  Serial.println("Baseline:");
  Serial.print("Left: "); Serial.println(baselineLeft, 3);
  Serial.print("Right: "); Serial.println(baselineRight, 3);
  Serial.print("Top: "); Serial.println(baselineTop, 3);
  Serial.print("Bottom: "); Serial.println(baselineBottom, 3);
  Serial.print("Horizontal Bias: "); Serial.println(HorizontalBias, 3);
  Serial.print("Vertical Bias: "); Serial.println(VerticalBias, 3);
}
void loop() {
  // Read LDRs
  float leftV  = analogRead(LDR_LEFT)  * VREF / ADC_MAX;
  float rightV = analogRead(LDR_RIGHT) * VREF / ADC_MAX;
  float topV = analogRead(LDR_TOP) * VREF / ADC_MAX;
  float bottomV = analogRead(LDR_BOTTOM) * VREF / ADC_MAX;
  // Calculate error
  float horizontalerror = (leftV - rightV) - HorizontalBias;
  float verticalerror = (topV - bottomV) - VerticalBias;
  // Servo control with deadband and scaling
  if (abs(horizontalerror) <= DEADBAND) {
    // Centered: snap to 90°
    AzservoAngle = 90;
  } else {
    AzservoAngle = 90 + int(horizontalerror * SCALE);
  }
  if(abs(verticalerror) <= DEADBAND) {
    // Centered: snap to 90°
    ElvservoAngle = 90;
  } else {
    ElvservoAngle = 90 + int(verticalerror * SCALE);
  }
// Constrain servo angle
  AzservoAngle = constrain(AzservoAngle, AZ_MIN, AZ_MAX);
  ElvservoAngle = constrain(ElvservoAngle, ELV_MIN, ELV_MAX);
// Write to servo
  azServo.write(AzservoAngle);
  elvServo.write(ElvservoAngle);
  // Debugging output
  Serial.print("L: "); Serial.print(leftV, 2);
  Serial.print(" R: "); Serial.print(rightV, 2);
  Serial.print(" Horizontal Diff: "); Serial.print(horizontalerror, 3);
  Serial.print(" Vertical Diff: "); Serial.print(verticalerror, 3);
  Serial.print(" Azimuth Servo: "); Serial.println(AzservoAngle);
  Serial.print(" Elevation Servo: "); Serial.println(ElvservoAngle);  

  delay(15); // update frequency around 66 Hz
}