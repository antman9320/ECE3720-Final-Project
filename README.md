# Traffic Light Simulator

ECE 3720 Final Project

**Group members:** Anthony Putrus, Mario Ivezaj, Justin Aquilina, David Hurlbert

# Link to Google Doc containing notes/ideas/project updates for the project
(https://docs.google.com/document/d/1CXY0ckCbr0uh1d2bbIugTZ3-Kkri9Xm0IxcwYGNHimU/edit?usp=sharing)


## Car motion and signal outputs (Mario Ivezaj)

Drives the six traffic-signal outputs and the two car motors, and makes
the cars obey the lights: go on green, slow on yellow, stop at the stop
line on red, brake for emergency vehicles.

Files: pins.h, traffic_types.h, clock.c/.h, inputs.c/.h, lights.c/.h,
motor.c/.h, car_motion.c/.h, signal_seq.c/.h, main.c

### Hardware other than the Dragon12 (this subsystem)
- 2 small DC gear motors, 3-6 V (on-board SN754410 H-bridge, T4 M1-M4)
- 12 LEDs (4 red, 4 yellow, 4 green) with 12 x 470 ohm resistors on PA0-PA5
- 2 IR reflective sensor modules, digital output, on PH0/PH1 (planned)
- Optional 5-9 V motor supply on VEXT

### Board setup
- Move the jumper from J24 to J18 (H-bridge power; disables the 7-segment display)
- All DIP switches (SW1) up
- Do not press the keypad while running (it shares Port A)

### Demo controls
- SW5 / SW4: hold = car at the N/S / E/W stop line
- SW3: end the current green early (stand-in for the camera)
- SW2: emergency vehicle on/off

### References
- EVBplus, Dragon12-Plus Trainer User's Manual: pin assignments, jumpers, PLL
  template (all files)
- MicroDigitalEd, DC Motor Control H-Bridge sample program: H-bridge pin
  mapping and PWM settings (motor.c)
- NXP AN2882, Using Real-Time Interrupt on HCS12: RTI setting (clock.c)
