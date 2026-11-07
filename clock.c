/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    clock.c
 * Purpose: Bus clock setup and a ~1 ms system tick from the Real-Time
 *          Interrupt (RTI). Everything else in the program is timed from
 *          this tick, so no busy-wait delay loops are needed.
 * Subsystem owner: Mario Ivezaj
 *
 * SOURCE / ATTRIBUTION
 *   - PLL settings (SYNR = 5, REFDV = 1 -> 48 MHz PLL, 24 MHz bus) follow the
 *     PLL template in the Dragon12-Plus User's Manual, chapter 7.
 *   - RTI setting RTICTL = 0x17 (8 MHz / (8 x 2^10)) and the CRG register bits
 *     match NXP application note AN2882, "Using Real-Time Interrupt on HCS12
 *     Microcontrollers".
 *
 * WHY THE RTI (and not an ECT timer channel)?
 *   The RTI runs from the 8 MHz crystal, so it does not depend on the PLL and
 *   it does not use up any of the timer channels on Port T that teammates may
 *   need later (PT0/PT1 are the rotary-encoder inputs).
 *
 *   RTI period = (RTR[3:0] + 1) * 2^(RTR[6:4] + 9) / OSCCLK
 *              = (7 + 1) * 2^(1 + 9) / 8 MHz = 1.024 ms   (RTICTL = 0x17)
 *   The 2.4 % error only stretches the light timings slightly (5 s -> 5.12 s).
 */
#include <hidef.h>        /* EnableInterrupts, _FEED_COP                    */
#include "derivative.h"
#include "clock.h"

static volatile unsigned int s_ms_ticks = 0;

void clock_init(void)
{
    /* Under the serial monitor the PLL is already running at 48 MHz, so only
     * set it up when it is not selected (stand-alone / simulator runs). This
     * avoids disturbing the monitor's SCI0 baud rate. */
    if ((CLKSEL & 0x80u) == 0u) {          /* PLLSEL = 0 ?                   */
        SYNR  = 0x05u;                     /* 2 * 8 MHz * (5+1)/(1+1) = 48   */
        REFDV = 0x01u;                     /* (template from the manual)     */
        PLLCTL |= 0x40u;                   /* PLLON                          */
        while ((CRGFLG & 0x08u) == 0u) {   /* wait for LOCK                  */
        }
        CLKSEL |= 0x80u;                   /* PLLSEL: bus = PLLCLK/2 = 24 MHz */
    }

    RTICTL = 0x17u;                        /* 1.024 ms real-time interrupt   */
    CRGINT |= 0x80u;                       /* RTIE: enable the RTI interrupt */
}

unsigned int clock_ms(void)
{
    /* A 16-bit read is one LDD instruction on the HCS12, so it is atomic. */
    return s_ms_ticks;
}

/* RTI interrupt service routine (vector 7 = Vrti).
 * Interrupt handlers must sit in non-banked memory. */
#pragma CODE_SEG __NEAR_SEG NON_BANKED
interrupt 7 void rti_isr(void)
{
    CRGFLG = 0x80u;                        /* clear RTIF by writing a 1      */
    s_ms_ticks++;
}
#pragma CODE_SEG DEFAULT
