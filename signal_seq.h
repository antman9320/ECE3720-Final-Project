/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    signal_seq.h
 * Purpose: Light sequencer used to drive and demo the car-motion subsystem.
 *          It runs a timed cycle and accepts two external commands, which
 *          are the hooks for the camera / Raspberry Pi logic:
 *            signal_request_switch()  - "lots of cars waiting on red,
 *                                        end the current green early"
 *            signal_set_emergency(on) - emergency vehicle: all red
 * Subsystem owner: Mario Ivezaj
 *
 * NOTE FOR THE TEAM: if the adaptive traffic logic replaces this file, keep
 * the same three outputs (signal_get, signal_emergency_active) so that
 * lights.c and car_motion.c keep working unchanged.
 */
#ifndef SIGNAL_SEQ_H
#define SIGNAL_SEQ_H

#include "traffic_types.h"

void          signal_init(void);
void          signal_update_10ms(void);
signal_t      signal_get(lane_t lane);
void          signal_request_switch(void);
void          signal_set_emergency(unsigned char on);
unsigned char signal_emergency_requested(void);
unsigned char signal_emergency_active(void);
unsigned char signal_debug_state(void);   /* phase number, for testing */

#endif /* SIGNAL_SEQ_H */
