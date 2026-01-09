# Dual-axis-Solar-Tracking-System (in progress)
## Project Overview
This project has the ultimate goal to track directional light using multiple LDR sensors and an ESP32.  
It was developed step by step, starting with basic sensor validation and then using control centered design that can driving actuators based on light direction.  
The final goal is a dual-axis (azimuth + elevation) tracking system similar to solar trackers or camera gimbals.

---

## Prototype 1: Quadrant Light Sensing with Visual Feedback

![IMG_7139](https://github.com/user-attachments/assets/5256b0e0-4c12-46d5-b00c-cc738f3974a2)

**Goal:**  
Confirm LDR behavior and determine light direction using a simple output (LEDs).

**Design:**  
- Four LDRs in a square (quadrant) layout  
- Each LDR mapped to an LED (up, down, left, right) to display light intensity  

**What worked:**  
- Correctly identified the quadrant with the strongest light  
- Visual LED feedback simplified debugging and validation  

**Limitations:**  
- Limited-resolution directional output
- No continuous angle information  
- Not suitable for smooth actuator control  

**Reason for redesign:**  
Needed a way to convert light differences into a continuous control signal.

---

## Prototype 2: Differential Light Sensing for Azimuth Control

![IMG_7142](https://github.com/user-attachments/assets/c53735fc-054a-4699-ac6a-25eee90b6ebe)

**Goal:**  
Enable smooth left–right tracking with continuous angular response.

**Design:**  
- Rearranged the LDRs into a “+” pattern  
- Calculated left–right sensor differences to control azimuth servo  

**Improvements over Prototype 1:**  
- Continuous response instead of limited directional input  
- Better suited for servo control  
- Clear separation between sensing and actuation logic  

**Observed limitations:**  
- Sensitivity to noise near neutral  
- Potential for oscillation without filtering or deadband  

**Outcome:**  
Established a functional 1-axis tracking system and laid groundwork for dual-axis expansion.

---

## Planned Prototype 3: Dual-Axis (Azimuth + Elevation) Tracking
**Goal:**  
Extend the system to full 2-axis directional tracking.

**Planned Design:**  
- Left–right differential → azimuth control  
- Top–bottom differential → elevation control  
- Drive two independent servos  

**Expected Challenges:**  
- Increased sensor noise sensitivity  
- Mechanical limits and servo dead zones  

**Planned Improvements:**  
- Deadband around neutral to reduce jitter  
- Low-pass filtering of ADC readings  
- Optional PID control for smoother convergence  

---

## Results and Observations
- Sensor layout strongly impacts control quality  
- Differential sensing enables proportional control  
- Iterative prototyping improved stability and reduced complexity  

---

## Future Improvements
- Implement full PID control on both axes  
- Improve mechanical rigidity and damping  
- Replace breadboard wiring with a custom PCB  
- Optimize sensor placement for better axis decoupling  
