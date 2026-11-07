/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    clock.h
 * Purpose: Bus clock setup and a ~1 ms system tick from the Real-Time
 *          Interrupt (RTI).
 * Subsystem owner: Mario Ivezaj
 */
#ifndef CLOCK_H
#define CLOCK_H

void         clock_init(void);   /* PLL to 24 MHz bus (if needed), start RTI */
unsigned int clock_ms(void);     /* tick counter, 1.024 ms per count, wraps  */

#endif /* CLOCK_H */
