/*
 * led_control.c
 *
 *  Created on: 11. 3. 2021
 *      Author: zmatkar
 */


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/rom_map.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "pinout.h"
#include "display/led_control.h"

void LedSetDirect(uint16_t r, uint16_t g, uint16_t b)
{
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, r);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_4, g);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, b);
}

void LedSet(rgb_led_s led)
{
    uint16_t period = PWMGenPeriodGet(PWM1_BASE, PWM_GEN_3)-1;
    static float intens = 0.1;

    if ((led.i <= 100.0) && (led.i >= 0.0)) intens = led.i/100.0;

    period *= intens;

    if ((led.r >= 0.0) && (led.r <= 100.0) )
    {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, led.r*period/100);
    }
    if ((led.g >= 0.0) && (led.g <= 100.0) )
    {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_4, led.g*period/100); // green
    }
    if ((led.b >= 0.0) && (led.b <= 100.0) )
    {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, led.b*period/100);
    }
}

#define SET_BL_DIRECT(x) PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, x);


void LedSetBacklight(float bl)
{
    uint16_t period = PWMGenPeriodGet(PWM1_BASE, PWM_GEN_3);

    if ((bl >= 0.0) && (bl <= 100.0) )
    {
        PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, bl*(period-1)/100);
    }
}

void LedInit(void)
{
    //
    // Enable the PWM1 peripheral
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);

    // Wait for the PWM0 module to be ready.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM1))
    {
    }

    //
    // Configure the PWM generator for count down mode with immediate updates
    // to the parameters. ENable DEBUG run.
    //
    PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC | PWM_GEN_MODE_DBG_RUN);
    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC | PWM_GEN_MODE_DBG_RUN);

    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, 400);
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, 400);

    PWMOutputInvert(PWM1_BASE, PWM_OUT_4_BIT | PWM_OUT_6_BIT, false);

    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_4, 10); // green
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, 10); // red
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, 10); // blue
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, 399);
    PWMGenEnable(PWM1_BASE, PWM_GEN_2);
    PWMGenEnable(PWM1_BASE, PWM_GEN_3);

    PWMOutputState(PWM1_BASE, (PWM_OUT_4_BIT | PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT), true);

}
