/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    main.c
 * Purpose: Milestone 2 demo program for the traffic-light outputs and car
 *          motion. Runs the light sequence, drives the signal LEDs and makes
 *          both motor-driven cars obey the lights.
 * Subsystem owner: Mario Ivezaj
 *
 * SOURCE / ATTRIBUTION
 *   - Program skeleton (hidef.h, derivative.h, EnableInterrupts, _FEED_COP)
 *     follows the default CodeWarrior HCS12 C project template.
 *
 * DEMO CONTROLS (Dragon12-Plus, all DIP switches UP)
 *   SW5 (PH0)  hold = a car is at the N/S stop line
 *   SW4 (PH1)  hold = a car is at the E/W stop line
 *   SW3 (PH2)  press = "many cars waiting": end the current green early
 *              (stands in for the camera / Raspberry Pi until that link works)
 *   SW2 (PH3)  press = emergency vehicle on / off
 *
 * TIMING
 *   The RTI gives a ~1 ms tick. Inputs are sampled every tick; the light
 *   sequencer and the car state machines run every 10 ticks. If the main
 *   loop ever falls behind, the while-loop below catches up tick by tick, so
 *   no time is lost.
 */
#include <hidef.h>        /* EnableInterrupts, _FEED_COP */
#include "derivative.h"
#include "pins.h"
#include "clock.h"
#include "lights.h"
#include "motor.h"
#include "inputs.h"
#include "car_motion.h"
#include "signal_seq.h"

static void board_leds_init(void)
{
#if BOARD_LEDS_ON
    DDRJ |= 0x02u;                 /* PJ1 output                          */
    PTJ  &= (unsigned char)~0x02u; /* PJ1 = 0 enables the PB0-PB7 LEDs    */
#endif
}

static void wait_ms(unsigned int ms)
{
    unsigned int start = clock_ms();
    while ((unsigned int)(clock_ms() - start) < ms) {
        _FEED_COP();
    }
}

void main(void)
{
    unsigned int  last_tick;
    unsigned int  now;
    unsigned char sub10 = 0u;
    signal_t      ns;
    signal_t      ew;

    clock_init();
    lights_init();        /* all red   */
    motor_init();         /* motors off */
    inputs_init();
    car_init();
    signal_init();
    board_leds_init();

    EnableInterrupts;

    /* Lamp test: every signal LED on for 1 s so wiring faults are obvious. */
    lights_set_raw(LIGHT_ALL_MASK);
    wait_ms(1000u);
    lights_set(SIG_RED, SIG_RED);

    last_tick = clock_ms();

    for (;;) {
        _FEED_COP();
        now = clock_ms();

        while (last_tick != now) {
            last_tick++;
            inputs_sample_1ms();

            sub10++;
            if (sub10 >= 10u) {
                sub10 = 0u;

                /* Stand-ins for the camera / Raspberry Pi commands. */
                if (inputs_take_press(IN_REQUEST)) {
                    signal_request_switch();
                }
                if (inputs_take_press(IN_EMERGENCY)) {
                    signal_set_emergency((unsigned char)!signal_emergency_requested());
                }

                signal_update_10ms();
                ns = signal_get(LANE_NS);
                ew = signal_get(LANE_EW);
                lights_set(ns, ew);
                car_update_10ms(ns, ew, signal_emergency_active());
            }
        }
    }
}
