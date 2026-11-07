/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    inputs.c
 * Purpose: Debounced reading of the Port H inputs (stop-line sensors and
 *          the stand-in "request" and "emergency" buttons).
 * Subsystem owner: Mario Ivezaj
 *
 * SOURCE / ATTRIBUTION
 *   - Counter-based debouncing is a standard textbook technique; no code was
 *     copied from a specific source.
 *
 * Each input is sampled every 1 ms. Its debounced value only changes after
 * the raw pin has disagreed with it for IN_DEBOUNCE_MS samples in a row,
 * which filters out switch bounce and IR-sensor flicker.
 * Values are converted to "1 = active" here, so the rest of the program never
 * has to think about active-low wiring.
 */
#include "pins.h"
#include "inputs.h"

static unsigned char s_stable = 0u;   /* debounced state, 1 = active      */
static unsigned char s_presses = 0u;  /* latched inactive->active edges   */
static unsigned char s_count[8];      /* per-bit disagreement counters    */

void inputs_init(void)
{
    unsigned char i;

    IN_DDR      &= (unsigned char)~IN_MASK;  /* inputs                       */
    IN_PULL_EN  |= IN_MASK;                  /* enable internal pull devices */
    IN_PULL_SEL &= (unsigned char)~IN_MASK;  /* ... as pull-UPs              */

    s_stable  = 0u;
    s_presses = 0u;
    for (i = 0u; i < 8u; i++) {
        s_count[i] = 0u;
    }
}

void inputs_sample_1ms(void)
{
    unsigned char raw;
    unsigned char i;
    unsigned char bit;

#if IN_ACTIVE_LOW
    raw = (unsigned char)~IN_PORT;
#else
    raw = IN_PORT;
#endif
    raw &= IN_MASK;

    for (i = 0u; i < 8u; i++) {
        bit = (unsigned char)(1u << i);
        if ((IN_MASK & bit) == 0u) {
            continue;
        }
        if ((raw & bit) != (s_stable & bit)) {
            s_count[i]++;
            if (s_count[i] >= IN_DEBOUNCE_MS) {
                s_stable ^= bit;              /* accept the new level */
                s_count[i] = 0u;
                if ((s_stable & bit) != 0u) {
                    s_presses |= bit;         /* remember the press   */
                }
            }
        } else {
            s_count[i] = 0u;                  /* bounce: start over   */
        }
    }
}

unsigned char inputs_active(unsigned char mask)
{
    return (unsigned char)((s_stable & mask) != 0u);
}

unsigned char inputs_take_press(unsigned char mask)
{
    unsigned char hit = (unsigned char)((s_presses & mask) != 0u);
    s_presses &= (unsigned char)~mask;
    return hit;
}
