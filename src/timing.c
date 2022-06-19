/*
 * timing.c
 *
 *  Created on: 22. 9. 2021
 *      Author: zmatkar
 */

#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "inc/hw_memmap.h"
#include "src/timing.h"

#define T_TIMER (WTIMER0_BASE)
#define T_TIMER_SYS (SYSCTL_PERIPH_WTIMER0)

void TimingStart(timing_t* s){
    s->start = TimerValueGet64(T_TIMER);
}

void TimingEnd(timing_t* s){
    s->elapsed = labs(TimerValueGet64(T_TIMER) - s->start)*0.0125;
}

void TimingInit(void)
{
    SysCtlPeripheralEnable(T_TIMER_SYS);

    while(!SysCtlPeripheralReady(T_TIMER_SYS)){

    };
    TimerConfigure(T_TIMER, TIMER_CFG_PERIODIC);
    //TimerLoadSet(T_TIMER, TIMER_A, 0xFFFFFFFF);
    TimerEnable(T_TIMER, TIMER_BOTH);
}

