/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    lights.c
 * Purpose: Drives the red/yellow/green signal LEDs for both directions on
 *          Port A, with a "conflict monitor" safety check.
 * Subsystem owner: Mario Ivezaj
 *
 * SOURCE / ATTRIBUTION
 *   - The conflict-monitor idea (force all-red if crossing directions would
 *     both be allowed to move) is modeled on the conflict monitors used in
 *     real traffic signal cabinets.
 *
 * Only the six signal bits of Port A are ever changed (read-modify-write),
 * so PA6/PA7 stay free for other code.
 */
#include "pins.h"
#include "lights.h"

static unsigned char s_conflicts = 0;

static unsigned char ns_bits(signal_t s)
{
    if (s == SIG_GREEN)  { return LIGHT_NS_GREEN;  }
    if (s == SIG_YELLOW) { return LIGHT_NS_YELLOW; }
    return LIGHT_NS_RED;
}

static unsigned char ew_bits(signal_t s)
{
    if (s == SIG_GREEN)  { return LIGHT_EW_GREEN;  }
    if (s == SIG_YELLOW) { return LIGHT_EW_YELLOW; }
    return LIGHT_EW_RED;
}

void lights_init(void)
{
    LIGHT_DDR |= LIGHT_ALL_MASK;           /* signal pins are outputs        */
    lights_set(SIG_RED, SIG_RED);          /* power up in the safe state     */
}

void lights_set(signal_t ns, signal_t ew)
{
    unsigned char pattern;

    /* Conflict monitor: crossing directions may never both show a
     * non-red aspect. If the caller asks for that, fail safe to all red. */
    if ((ns != SIG_RED) && (ew != SIG_RED)) {
        ns = SIG_RED;
        ew = SIG_RED;
        if (s_conflicts < 255u) {
            s_conflicts++;
        }
    }

    pattern = (unsigned char)(ns_bits(ns) | ew_bits(ew));
    LIGHT_PORT = (unsigned char)((LIGHT_PORT & (unsigned char)~LIGHT_ALL_MASK) | pattern);
}

void lights_set_raw(unsigned char pattern)
{
    LIGHT_PORT = (unsigned char)((LIGHT_PORT & (unsigned char)~LIGHT_ALL_MASK)
                                 | (pattern & LIGHT_ALL_MASK));
}

unsigned char lights_conflict_count(void)
{
    return s_conflicts;
}
