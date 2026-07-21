#include <Arduino.h>

// LDR inputs
const int INPUT_1 = 36; // V1
const int INPUT_2 = 39; // V2
const int INPUT_3 = 34; // V3
const int INPUT_4 = 35; // V4
// LED outputs
const int LED_1 = 32;
const int LED_2 = 33;
const int LED_3 = 25;
const int LED_4 = 26;
// PWM constants
const int PWM_MAX = 255;  // 8-bit PWM
// Baseline voltages
const float BASELINE[4] = {2.23, 2.26, 2.13, 2.06};
const float THRESHOLD_DELTA = 0.7; // +0.7 V above baseline to react
void setup() {
  Serial.begin(115200);
  // ADC setup
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  // PWM setup
  ledcAttach(LED_1, 5000, 8);
  ledcAttach(LED_2, 5000, 8);
  ledcAttach(LED_3, 5000, 8);
  ledcAttach(LED_4, 5000, 8);
}
void loop() {
  // Read LDR voltages
  float v[4];
  v[0] = analogRead(INPUT_1) * 3.3 / 4095.0;
  v[1] = analogRead(INPUT_2) * 3.3 / 4095.0;
  v[2] = analogRead(INPUT_3) * 3.3 / 4095.0;
  v[3] = analogRead(INPUT_4) * 3.3 / 4095.0;

  // Compute voltage above baseline + threshold
  float delta[4];
  for (int i = 0; i < 4; i++) {
    delta[i] = v[i] - BASELINE[i] - THRESHOLD_DELTA;
    if (delta[i] < 0) delta[i] = 0; // only react to positive change
  }
  // LED power
  int ledPower[4] = {0, 0, 0, 0};
  // Map LDRs to LEDs
  // V1 → LED1 & LED2
  ledPower[0] += delta[0] / THRESHOLD_DELTA * PWM_MAX;
  ledPower[1] += delta[0] / THRESHOLD_DELTA * PWM_MAX;

  // V2 → LED1 & LED3
  ledPower[0] += delta[1] / THRESHOLD_DELTA * PWM_MAX;
  ledPower[2] += delta[1] / THRESHOLD_DELTA * PWM_MAX;

  // V3 → LED4 & LED2
  ledPower[3] += delta[2] / THRESHOLD_DELTA * PWM_MAX;
  ledPower[1] += delta[2] / THRESHOLD_DELTA * PWM_MAX;

  // V4 → LED4 & LED3
  ledPower[3] += delta[3] / THRESHOLD_DELTA * PWM_MAX;
  ledPower[2] += delta[3] / THRESHOLD_DELTA * PWM_MAX;

  // Clamp PWM to 0–255
  for (int i = 0; i < 4; i++) {
    if (ledPower[i] > PWM_MAX) ledPower[i] = PWM_MAX;
    if (ledPower[i] < 0) ledPower[i] = 0;
  }
  ledcWrite(LED_1, ledPower[0]);
  ledcWrite(LED_2, ledPower[1]);
  ledcWrite(LED_3, ledPower[2]);
  ledcWrite(LED_4, ledPower[3]);
  // Debug output
  Serial.print("V1: "); Serial.print(v[0], 2);
  Serial.print("  V2: "); Serial.print(v[1], 2);
  Serial.print("  V3: "); Serial.print(v[2], 2);
  Serial.print("  V4: "); Serial.println(v[3], 2);
  delay(50); // 20 updates per second
}
