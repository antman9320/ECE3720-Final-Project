/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    signal_seq.c
 * Purpose: Light sequencer used to drive and demo the car-motion subsystem.
 * Subsystem owner: Mario Ivezaj
 *
 * SOURCE / ATTRIBUTION
 *   - Phase order (green -> yellow -> all-red clearance -> other green) is
 *     the standard two-phase signal sequence; the min/max green and the
 *     emergency pre-emption behaviour were designed for this project.
 *
 * PHASES
 *   STARTUP (all red 1 s) -> NS green -> NS yellow -> all red
 *                         -> EW green -> EW yellow -> all red -> NS green ...
 *   A green lasts T_GREEN_MAX, or ends early (but never before T_GREEN_MIN)
 *   when signal_request_switch() is called.
 *
 * EMERGENCY PRE-EMPTION
 *   A green that is showing goes to yellow first (never straight from green
 *   to red), then every direction holds red until the emergency is cleared,
 *   then 1 s of all-red clearance, then service continues with the other
 *   direction.
 */
#include "pins.h"
#include "signal_seq.h"

typedef enum {
    SEQ_STARTUP = 0,
    SEQ_NS_GREEN,
    SEQ_NS_YELLOW,
    SEQ_NS_CLEAR,        /* all red after N/S */
    SEQ_EW_GREEN,
    SEQ_EW_YELLOW,
    SEQ_EW_CLEAR,        /* all red after E/W */
    SEQ_EMERG_YELLOW,    /* emergency: finishing a yellow */
    SEQ_EMERG_HOLD,      /* emergency: all red            */
    SEQ_EMERG_RELEASE    /* all-red clearance afterwards  */
} seq_state_t;

static seq_state_t   s_state;
static unsigned int  s_timer;        /* 10 ms ticks in the current phase */
static unsigned char s_request;      /* switch requested                 */
static unsigned char s_emerg_req;    /* emergency requested              */
static lane_t        s_yellow_lane;  /* lane showing yellow in EMERG_YELLOW */
static lane_t        s_next_green;   /* lane to serve next               */

static void enter(seq_state_t st)
{
    s_state = st;
    s_timer = 0u;
}

static void enter_green(lane_t lane)
{
    s_request = 0u;                    /* a new green starts fresh */
    if (lane == LANE_NS) {
        enter(SEQ_NS_GREEN);
        s_next_green = LANE_EW;
    } else {
        enter(SEQ_EW_GREEN);
        s_next_green = LANE_NS;
    }
}

void signal_init(void)
{
    s_request     = 0u;
    s_emerg_req   = 0u;
    s_yellow_lane = LANE_NS;
    s_next_green  = LANE_NS;
    enter(SEQ_STARTUP);
}

void signal_request_switch(void)
{
    s_request = 1u;
}

void signal_set_emergency(unsigned char on)
{
    s_emerg_req = (unsigned char)(on ? 1u : 0u);
}

unsigned char signal_emergency_requested(void)
{
    return s_emerg_req;
}

unsigned char signal_emergency_active(void)
{
    return (unsigned char)(s_emerg_req
                           || (s_state == SEQ_EMERG_YELLOW)
                           || (s_state == SEQ_EMERG_HOLD));
}

unsigned char signal_debug_state(void)
{
    return (unsigned char)s_state;
}

static unsigned char green_done(void)
{
    if (s_timer >= T_GREEN_MAX) {
        return 1u;
    }
    if (s_request && (s_timer >= T_GREEN_MIN)) {
        return 1u;
    }
    return 0u;
}

void signal_update_10ms(void)
{
    if (s_timer < 0xFFFFu) {
        s_timer++;
    }

    /* ---------------- emergency pre-emption ---------------- */
    if (s_emerg_req) {
        switch (s_state) {
        case SEQ_NS_GREEN:
            s_yellow_lane = LANE_NS;
            enter(SEQ_EMERG_YELLOW);           /* full yellow time */
            break;
        case SEQ_EW_GREEN:
            s_yellow_lane = LANE_EW;
            enter(SEQ_EMERG_YELLOW);
            break;
        case SEQ_NS_YELLOW:
            s_yellow_lane = LANE_NS;
            s_state = SEQ_EMERG_YELLOW;        /* keep the yellow timer */
            break;
        case SEQ_EW_YELLOW:
            s_yellow_lane = LANE_EW;
            s_state = SEQ_EMERG_YELLOW;
            break;
        case SEQ_EMERG_YELLOW:
            if (s_timer >= T_YELLOW) {
                enter(SEQ_EMERG_HOLD);
            }
            break;
        case SEQ_EMERG_HOLD:
            break;                             /* stay all red */
        default:                               /* already all red */
            enter(SEQ_EMERG_HOLD);
            break;
        }
        return;
    }

    /* ---------------- normal cycle ---------------- */
    switch (s_state) {
    case SEQ_STARTUP:
        if (s_timer >= T_ALL_RED) {
            enter_green(s_next_green);
        }
        break;

    case SEQ_NS_GREEN:
        if (green_done()) {
            enter(SEQ_NS_YELLOW);
        }
        break;
    case SEQ_NS_YELLOW:
        if (s_timer >= T_YELLOW) {
            enter(SEQ_NS_CLEAR);
        }
        break;
    case SEQ_NS_CLEAR:
        if (s_timer >= T_ALL_RED) {
            enter_green(LANE_EW);
        }
        break;

    case SEQ_EW_GREEN:
        if (green_done()) {
            enter(SEQ_EW_YELLOW);
        }
        break;
    case SEQ_EW_YELLOW:
        if (s_timer >= T_YELLOW) {
            enter(SEQ_EW_CLEAR);
        }
        break;
    case SEQ_EW_CLEAR:
        if (s_timer >= T_ALL_RED) {
            enter_green(LANE_NS);
        }
        break;

    case SEQ_EMERG_YELLOW:                     /* cancelled mid-yellow */
        if (s_timer >= T_YELLOW) {
            enter(SEQ_EMERG_RELEASE);
        }
        break;
    case SEQ_EMERG_HOLD:
        enter(SEQ_EMERG_RELEASE);
        break;
    case SEQ_EMERG_RELEASE:
        if (s_timer >= T_ALL_RED) {
            enter_green(s_next_green);
        }
        break;

    default:
        enter(SEQ_STARTUP);
        break;
    }
}

signal_t signal_get(lane_t lane)
{
    switch (s_state) {
    case SEQ_NS_GREEN:     return (lane == LANE_NS) ? SIG_GREEN  : SIG_RED;
    case SEQ_NS_YELLOW:    return (lane == LANE_NS) ? SIG_YELLOW : SIG_RED;
    case SEQ_EW_GREEN:     return (lane == LANE_EW) ? SIG_GREEN  : SIG_RED;
    case SEQ_EW_YELLOW:    return (lane == LANE_EW) ? SIG_YELLOW : SIG_RED;
    case SEQ_EMERG_YELLOW: return (lane == s_yellow_lane) ? SIG_YELLOW : SIG_RED;
    default:               return SIG_RED;
    }
}
