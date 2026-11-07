/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    motor.h
 * Purpose: PWM speed control of the two car motors through the on-board
 *          SN754410 H-bridge.
 * Subsystem owner: Mario Ivezaj
 */
#ifndef MOTOR_H
#define MOTOR_H

#include "traffic_types.h"   /* lane_t: LANE_NS -> motor 1, LANE_EW -> motor 2 */

void          motor_init(void);
void          motor_forward(lane_t lane, unsigned char duty_pct); /* 0-100 */
void          motor_coast(lane_t lane);   /* bridge off, car rolls to a stop */
void          motor_brake(lane_t lane);   /* both terminals to GND: quick stop */
unsigned char motor_get_duty(lane_t lane);

#endif /* MOTOR_H */
