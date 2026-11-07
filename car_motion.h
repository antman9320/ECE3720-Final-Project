/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    car_motion.h
 * Purpose: Traffic simulation - makes the motor-driven cars obey the lights.
 * Subsystem owner: Mario Ivezaj
 *
 * HOW OTHER MODULES USE THIS
 *   Call car_update_10ms() every 10 ms with whatever the lights are showing.
 *   car_get_pass_count() tells the traffic logic how many cars actually went
 *   through on green; the camera can then compare that with the number of
 *   cars waiting on red.
 */
#ifndef CAR_MOTION_H
#define CAR_MOTION_H

#include "traffic_types.h"

typedef enum {
    CAR_STOPPED = 0,  /* not driven; waits for green                      */
    CAR_ACCEL,        /* green: soft-start ramp up to cruise speed         */
    CAR_CRUISE,       /* green: running at cruise speed                    */
    CAR_SLOWING,      /* yellow: slowing to approach speed                 */
    CAR_CREEP,        /* red: rolling slowly up to the stop line           */
    CAR_HELD,         /* red: stopped at the line (braked) until green     */
    CAR_EMERGENCY     /* emergency vehicle: every car braked immediately   */
} car_state_t;

void          car_init(void);
void          car_update_10ms(signal_t ns, signal_t ew, unsigned char emergency);
car_state_t   car_get_state(lane_t lane);
unsigned char car_get_duty(lane_t lane);       /* current speed command, %  */
unsigned int  car_get_pass_count(lane_t lane); /* cars that crossed the line */
void          car_clear_pass_count(lane_t lane);

#endif /* CAR_MOTION_H */
