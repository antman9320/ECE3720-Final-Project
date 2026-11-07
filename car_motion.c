/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    car_motion.c
 * Purpose: Traffic simulation - makes the motor-driven cars obey the lights.
 *          One state machine per direction (N/S and E/W).
 * Subsystem owner: Mario Ivezaj
 *
 * SOURCE / ATTRIBUTION
 *   - The state machine, ramp/kick speed profile, stop-line creep with a
 *     timeout, and pass counting were designed for this project; no code was
 *     copied from another project.
 *
 * STATE MACHINE (per lane, evaluated every 10 ms)
 *
 *   any state --emergency--> EMERGENCY --cleared--> STOPPED
 *
 *   STOPPED / HELD --green--> ACCEL --reached cruise--> CRUISE
 *   ACCEL / CRUISE --yellow--> SLOWING --green--> ACCEL
 *   ACCEL / CRUISE / SLOWING --red--> CREEP   (or HELD if already at line)
 *   CREEP --car reaches stop line, or 3 s timeout--> HELD
 *   CREEP --green--> ACCEL
 *
 * SPEED PROFILE
 *   Speed changes are rate-limited (soft start / soft stop) so the cars look
 *   realistic and the motor inrush current stays low. Because small DC
 *   motors will not turn below roughly 30 % duty, a start "kicks" straight
 *   to DUTY_MIN_MOVE and ramps up from there. HELD and EMERGENCY use the
 *   H-bridge brake so the car stops right at the line.
 *
 * SAFETY
 *   CREEP has a timeout: if a stop-line sensor fails, the car still stops
 *   after CREEP_TIMEOUT_10MS instead of driving through the red light.
 */
#include "pins.h"
#include "inputs.h"
#include "motor.h"
#include "car_motion.h"

typedef struct {
    car_state_t   state;
    unsigned char duty;         /* speed command after ramping, percent    */
    unsigned int  creep_timer;  /* 10 ms ticks spent in CREEP              */
    unsigned char at_line;      /* last stop-line reading (1 = car there)  */
    unsigned int  passed;       /* cars that left the line on green/yellow */
} car_t;

static car_t s_car[LANE_COUNT];

#if CAR_USE_STOPLINE_SENSORS
static unsigned char stopline_mask(lane_t lane)
{
    return (lane == LANE_NS) ? IN_STOPLINE_NS : IN_STOPLINE_EW;
}
#endif

/* Move 'duty' one step toward 'target' (soft start / soft stop). */
static unsigned char ramp(unsigned char duty, unsigned char target)
{
    if (duty < target) {
        if (duty < DUTY_MIN_MOVE) {
            duty = DUTY_MIN_MOVE;                 /* kick past static friction */
            if (duty > target) {
                duty = target;
            }
        } else if ((unsigned char)(target - duty) > RAMP_UP_STEP) {
            duty = (unsigned char)(duty + RAMP_UP_STEP);
        } else {
            duty = target;
        }
    } else if (duty > target) {
        if ((unsigned char)(duty - target) > RAMP_DOWN_STEP) {
            duty = (unsigned char)(duty - RAMP_DOWN_STEP);
        } else {
            duty = target;
        }
        if (duty < DUTY_MIN_MOVE) {
            duty = target;   /* the motor stalls below this anyway */
        }
    }
    return duty;
}

static unsigned char is_moving_state(car_state_t s)
{
    return (unsigned char)((s == CAR_ACCEL) || (s == CAR_CRUISE) || (s == CAR_SLOWING));
}

static void start_red(car_t *c, unsigned char line_now)
{
#if CAR_USE_STOPLINE_SENSORS
    if (line_now) {
        c->state = CAR_HELD;           /* already at the line: stop here */
    } else {
        c->state = CAR_CREEP;          /* roll up to the line            */
        c->creep_timer = 0u;
    }
#else
    (void)line_now;
    c->state = CAR_HELD;
#endif
}

static void update_lane(lane_t lane, signal_t sig, unsigned char emergency)
{
    car_t *c = &s_car[lane];
    unsigned char line_now;
    unsigned char target;

#if CAR_USE_STOPLINE_SENSORS
    line_now = inputs_active(stopline_mask(lane));
#else
    line_now = 0u;
#endif

    /* A car leaving the stop-line sensor while traffic is moving has
     * gone through the intersection: count it. */
    if (c->at_line && !line_now && is_moving_state(c->state)) {
        c->passed++;
    }
    c->at_line = line_now;

    /* ---- state transitions ---- */
    if (emergency) {
        c->state = CAR_EMERGENCY;
    } else {
        switch (c->state) {
        case CAR_EMERGENCY:
            c->state = CAR_STOPPED;    /* resume normal rules next tick */
            break;

        case CAR_STOPPED:
        case CAR_HELD:
            if (sig == SIG_GREEN) {
                c->state = CAR_ACCEL;
            }
            break;

        case CAR_ACCEL:
        case CAR_CRUISE:
        case CAR_SLOWING:
            if (sig == SIG_RED) {
                start_red(c, line_now);
            } else if (sig == SIG_YELLOW) {
                c->state = CAR_SLOWING;
            } else if (c->state == CAR_SLOWING) {
                c->state = CAR_ACCEL;  /* back to green (rare) */
            }
            break;

        case CAR_CREEP:
            if (sig == SIG_GREEN) {
                c->state = CAR_ACCEL;
            } else if (line_now) {
                c->state = CAR_HELD;
            } else {
                c->creep_timer++;
                if (c->creep_timer >= CREEP_TIMEOUT_10MS) {
                    c->state = CAR_HELD;   /* fail safe */
                }
            }
            break;

        default:
            c->state = CAR_STOPPED;
            break;
        }
    }

    /* ---- outputs ---- */
    if ((c->state == CAR_EMERGENCY) || (c->state == CAR_HELD)) {
        c->duty = 0u;
        motor_brake(lane);
        return;
    }

    switch (c->state) {
    case CAR_ACCEL:
    case CAR_CRUISE:  target = DUTY_CRUISE; break;
    case CAR_SLOWING: target = DUTY_SLOW;   break;
    case CAR_CREEP:   target = DUTY_CREEP;  break;
    default:          target = 0u;          break;
    }

    c->duty = ramp(c->duty, target);
    if (c->duty == 0u) {
        motor_coast(lane);
    } else {
        motor_forward(lane, c->duty);
    }

    if ((c->state == CAR_ACCEL) && (c->duty >= DUTY_CRUISE)) {
        c->state = CAR_CRUISE;
    }
}

void car_init(void)
{
    unsigned char i;
    for (i = 0u; i < (unsigned char)LANE_COUNT; i++) {
        s_car[i].state       = CAR_STOPPED;
        s_car[i].duty        = 0u;
        s_car[i].creep_timer = 0u;
        s_car[i].at_line     = 0u;
        s_car[i].passed      = 0u;
        motor_coast((lane_t)i);
    }
}

void car_update_10ms(signal_t ns, signal_t ew, unsigned char emergency)
{
    update_lane(LANE_NS, ns, emergency);
    update_lane(LANE_EW, ew, emergency);
}

car_state_t car_get_state(lane_t lane)
{
    return s_car[lane].state;
}

unsigned char car_get_duty(lane_t lane)
{
    return s_car[lane].duty;
}

unsigned int car_get_pass_count(lane_t lane)
{
    return s_car[lane].passed;
}

void car_clear_pass_count(lane_t lane)
{
    s_car[lane].passed = 0u;
}
