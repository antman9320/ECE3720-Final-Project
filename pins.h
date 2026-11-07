/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    pins.h
 * Purpose: ONE place for every pin assignment and tuning constant used by the
 *          traffic-light output and car-motion subsystem. If the team rewires
 *          something, change it here and nowhere else.
 * Subsystem owner: Mario Ivezaj (traffic light outputs + car motion)
 *
 * SOURCE / ATTRIBUTION
 *   - Dragon12-Plus pin facts (PB0/PB1 -> 1A/2A, PP0 -> EN12, PB2/PB3 -> 3A/4A,
 *     PP1 -> EN34, PJ1 = LED enable, PH0-PH3 = pushbuttons SW5-SW2) come from
 *     the Dragon12-Plus User's Manual (EVBplus) and the MicroDigitalEd
 *     "DC Motor Control H-Bridge" sample program.
 *   - Choice of Port A for the signal LEDs, the input assignments and all
 *     tuning constants are original to this project.
 *
 * REQUIRED BOARD JUMPERS
 *   - Move the jumper from J24 (7-segment driver power) to J18 (H-bridge
 *     power). Only one of them may have a jumper. The 7-segment display
 *     cannot be used while the motors are in use (PP0/PP1 are shared).
 *   - Motor supply: on-board VIN, or an external 5-9 V supply on the VEXT/GND
 *     pins of terminal block T4 (move the jumper next to VEXT to the lower
 *     two pins). Do not exceed 9 V.
 *   - Put all eight DIP switches (SW1) in the UP/open position so that the
 *     pushbuttons on PH0-PH3 read correctly.
 */
#ifndef PINS_H
#define PINS_H

#include "derivative.h"   /* CodeWarrior register definitions (MC9S12DG256) */

/* ------------------------------------------------------------------------ */
/* Clock                                                                    */
/* ------------------------------------------------------------------------ */
/* 8 MHz crystal, PLL -> 48 MHz, bus = 24 MHz (same as the serial monitor). */
#define OSC_CLOCK_HZ     8000000UL
#define BUS_CLOCK_HZ    24000000UL

/* ------------------------------------------------------------------------ */
/* Traffic-signal LEDs on Port A (external LEDs on the intersection model)  */
/* Each output drives the two heads that face opposite approaches (for      */
/* example N and S) - one LED + one 470 ohm resistor per head, in parallel. */
/* About 6 mA per LED, 12 mA per pin, within the HCS12 pin rating.          */
/* Port A is also wired to the on-board keypad: do not press keypad keys    */
/* while the program is running.                                            */
/* ------------------------------------------------------------------------ */
#define LIGHT_PORT        PORTA
#define LIGHT_DDR         DDRA
#define LIGHT_NS_RED      0x01u   /* PA0 */
#define LIGHT_NS_YELLOW   0x02u   /* PA1 */
#define LIGHT_NS_GREEN    0x04u   /* PA2 */
#define LIGHT_EW_RED      0x08u   /* PA3 */
#define LIGHT_EW_YELLOW   0x10u   /* PA4 */
#define LIGHT_EW_GREEN    0x20u   /* PA5 */
#define LIGHT_ALL_MASK    0x3Fu   /* PA6, PA7 left free for the team */

/* ------------------------------------------------------------------------ */
/* On-board SN754410 dual H-bridge (pin compatible with the L293D)          */
/*   Motor 1 = north/south lane car(s): terminals M1/M2 on T4               */
/*             1A = PB0, 2A = PB1, EN12 = PP0 = PWM channel 0               */
/*   Motor 2 = east/west lane car(s):   terminals M3/M4 on T4               */
/*             3A = PB2, 4A = PB3, EN34 = PP1 = PWM channel 1               */
/* PB0-PB3 also drive on-board LEDs 0-3, which act as direction indicators. */
/* ------------------------------------------------------------------------ */
#define HB_DIR_PORT       PORTB
#define HB_DIR_DDR        DDRB
#define HB_M1_IN_A        0x01u   /* PB0 -> 1A */
#define HB_M1_IN_B        0x02u   /* PB1 -> 2A */
#define HB_M2_IN_A        0x04u   /* PB2 -> 3A */
#define HB_M2_IN_B        0x08u   /* PB3 -> 4A */
#define HB_DIR_MASK       0x0Fu

#define PWM_CH_M1         0x01u   /* PWME/PWMPOL/... bit for channel 0 (PP0) */
#define PWM_CH_M2         0x02u   /* bit for channel 1 (PP1) */

/* Set to 1 if a car drives backwards (motor leads swapped). */
#define MOTOR1_REVERSED   0
#define MOTOR2_REVERSED   0

/* PWM clock: Clock A = bus / 2^PWM_PCKA. 24 MHz / 32 = 750 kHz.
 * With a period of 100 counts the PWM runs at 7.5 kHz and the duty
 * register is directly "percent on". */
#define PWM_PCKA          5u
#define PWM_PERIOD        100u

/* Set to 1 to light the on-board PB LEDs (PJ1 = 0). PB0-PB3 then show
 * the H-bridge direction inputs, which is handy during the demo. */
#define BOARD_LEDS_ON     1

/* ------------------------------------------------------------------------ */
/* Inputs on Port H (active low: pressed / sensor triggered = 0)            */
/* For Milestone 2 the on-board pushbuttons stand in for real sensors.      */
/* Later, an IR stop-line sensor with an open-collector output can be wired */
/* to the same pin (DIP switch for that pin must stay UP).                  */
/* ------------------------------------------------------------------------ */
#define IN_PORT           PTH
#define IN_DDR            DDRH
#define IN_PULL_EN        PERH
#define IN_PULL_SEL       PPSH
#define IN_STOPLINE_NS    0x01u   /* PH0 = SW5: car at N/S stop line         */
#define IN_STOPLINE_EW    0x02u   /* PH1 = SW4: car at E/W stop line         */
#define IN_REQUEST        0x04u   /* PH2 = SW3: "switch now" request (stands
                                     in for the camera / Raspberry Pi)       */
#define IN_EMERGENCY      0x08u   /* PH3 = SW2: emergency vehicle (toggle)   */
#define IN_MASK           0x0Fu
#define IN_ACTIVE_LOW     1
#define IN_DEBOUNCE_MS    10u     /* input must be stable this long          */

/* ------------------------------------------------------------------------ */
/* Car-motion tuning (duty values are percent, 0-100)                       */
/* ------------------------------------------------------------------------ */
#define CAR_USE_STOPLINE_SENSORS 1   /* 0 = stop as soon as the light is red */
#define DUTY_CRUISE       70u    /* green                                    */
#define DUTY_SLOW         45u    /* yellow                                   */
#define DUTY_CREEP        35u    /* red, rolling up to the stop line         */
#define DUTY_MIN_MOVE     30u    /* below this a small motor will not turn,  */
                                 /* so starts "kick" straight to this value  */
#define RAMP_UP_STEP       1u    /* percent added every 10 ms  (soft start)  */
#define RAMP_DOWN_STEP     2u    /* percent removed every 10 ms              */
#define CREEP_TIMEOUT_10MS 300u  /* 3 s: never creep past a red forever      */

/* ------------------------------------------------------------------------ */
/* Signal timing for the stand-in sequencer (units of 10 ms)                */
/* ------------------------------------------------------------------------ */
#define T_GREEN_MIN       500u   /* 5 s  */
#define T_GREEN_MAX      1500u   /* 15 s */
#define T_YELLOW          300u   /* 3 s  */
#define T_ALL_RED         100u   /* 1 s clearance */

#endif /* PINS_H */
