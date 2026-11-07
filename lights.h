/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    lights.h
 * Purpose: Drives the red/yellow/green signal LEDs for both directions.
 * Subsystem owner: Mario Ivezaj
 */
#ifndef LIGHTS_H
#define LIGHTS_H

#include "traffic_types.h"

void          lights_init(void);
void          lights_set(signal_t ns, signal_t ew);  /* with conflict check   */
void          lights_set_raw(unsigned char pattern); /* lamp test only        */
unsigned char lights_conflict_count(void);           /* times the check fired */

#endif /* LIGHTS_H */
