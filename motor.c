/*
 * Traffic Light Simulator - ECE 3720 Final Project
 * File:    motor.c
 * Purpose: PWM speed control of the two car motors through the on-board
 *          SN754410 H-bridge (pin compatible with the L293D).
 * Subsystem owner: Mario Ivezaj
 *
 * SOURCE / ATTRIBUTION
 *   - Pin mapping (PP0 = EN12, PB0/PB1 = 1A/2A, PP1 = EN34, PB2/PB3 = 3A/4A)
 *     and the PWM settings PWMPRCLK = 5 (bus/32) and PWMPER = 100 were taken
 *     from the MicroDigitalEd "DC Motor Control H-Bridge" Dragon12 sample
 *     program. That sample drives one motor with blocking delays; the
 *     two-motor driver, brake/coast modes and the API below are new.
 *
 * HOW THE H-BRIDGE IS USED (one half shown; the other is identical)
 *     EN (PWM)  1A  2A   Result
 *       duty     1   0   forward, speed set by the duty cycle
 *       100 %    0   0   brake: both motor leads tied to GND
 *        0 %     x   x   coast: outputs off (high impedance)
 */
#include "pins.h"
#include "motor.h"

static unsigned char s_duty[LANE_COUNT] = { 0u, 0u };

/* Direction-pin bits for each motor, swapped if the motor is reversed. */
static unsigned char fwd_bits(lane_t lane)
{
    if (lane == LANE_NS) {
#if MOTOR1_REVERSED
        return HB_M1_IN_B;
#else
        return HB_M1_IN_A;
#endif
    }
#if MOTOR2_REVERSED
    return HB_M2_IN_B;
#else
    return HB_M2_IN_A;
#endif
}

static unsigned char dir_mask(lane_t lane)
{
    return (lane == LANE_NS) ? (unsigned char)(HB_M1_IN_A | HB_M1_IN_B)
                             : (unsigned char)(HB_M2_IN_A | HB_M2_IN_B);
}

static void set_dir_bits(lane_t lane, unsigned char bits)
{
    HB_DIR_PORT = (unsigned char)((HB_DIR_PORT & (unsigned char)~dir_mask(lane)) | bits);
}

static void set_pwm(lane_t lane, unsigned char duty)
{
    if (duty > PWM_PERIOD) {
        duty = PWM_PERIOD;
    }
    if (lane == LANE_NS) {
        PWMDTY0 = duty;
    } else {
        PWMDTY1 = duty;
    }
}

void motor_init(void)
{
    /* Direction pins: outputs, everything low (bridge inputs 0/0). */
    HB_DIR_DDR  |= HB_DIR_MASK;
    HB_DIR_PORT &= (unsigned char)~HB_DIR_MASK;

    /* Configure PWM channels 0 and 1 while they are disabled. */
    PWME     &= (unsigned char)~(PWM_CH_M1 | PWM_CH_M2);
    PWMPOL   |= (PWM_CH_M1 | PWM_CH_M2);      /* high at start of period:    */
                                              /* duty register = on-time     */
    PWMCLK   &= (unsigned char)~(PWM_CH_M1 | PWM_CH_M2); /* use Clock A     */
    PWMPRCLK  = (unsigned char)((PWMPRCLK & 0x70u) | PWM_PCKA); /* keep B    */
    PWMCAE   &= (unsigned char)~(PWM_CH_M1 | PWM_CH_M2); /* left aligned    */
    PWMCTL   &= (unsigned char)~0x10u;        /* CON01 = 0: two 8-bit chans  */
    PWMPER0   = PWM_PERIOD;
    PWMPER1   = PWM_PERIOD;
    PWMDTY0   = 0u;
    PWMDTY1   = 0u;
    PWMCNT0   = 0u;                           /* any write resets the counter */
    PWMCNT1   = 0u;
    PWME     |= (PWM_CH_M1 | PWM_CH_M2);      /* start both channels         */

    s_duty[LANE_NS] = 0u;
    s_duty[LANE_EW] = 0u;
}

void motor_forward(lane_t lane, unsigned char duty_pct)
{
    if (duty_pct > 100u) {
        duty_pct = 100u;
    }
    if (duty_pct == 0u) {
        motor_coast(lane);
        return;
    }
    set_pwm(lane, duty_pct);          /* speed first, then direction pins */
    set_dir_bits(lane, fwd_bits(lane));
    s_duty[lane] = duty_pct;
}

void motor_coast(lane_t lane)
{
    set_pwm(lane, 0u);                /* EN low: bridge outputs off */
    set_dir_bits(lane, 0u);
    s_duty[lane] = 0u;
}

void motor_brake(lane_t lane)
{
    set_dir_bits(lane, 0u);           /* both inputs low ...                 */
    set_pwm(lane, PWM_PERIOD);        /* ... with EN fully on = short brake  */
    s_duty[lane] = 0u;                /* car is not being driven             */
}

unsigned char motor_get_duty(lane_t lane)
{
    return s_duty[lane];
}
