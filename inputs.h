/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    inputs.h
 * Purpose: Debounced reading of the Port H inputs (stop-line sensors and
 *          the stand-in "request" and "emergency" buttons).
 * Subsystem owner: Mario Ivezaj
 */
#ifndef INPUTS_H
#define INPUTS_H

void          inputs_init(void);
void          inputs_sample_1ms(void);              /* call once per tick  */
unsigned char inputs_active(unsigned char mask);    /* 1 = pressed/blocked */
unsigned char inputs_take_press(unsigned char mask);/* 1 once per press    */

#endif /* INPUTS_H */
