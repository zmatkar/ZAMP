/*
 * scheduler.c
 *
 *  Created on: 21. 8. 2017
 *      Author: kosan
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/rom_map.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/timer.h"
#include "driverlib/ssi.h"
#include "driverlib/fpu.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "scheduler.h"

static task_t task_array[TASKS_MAX] = {NULL};

static uint16_t task_num = 0;
static volatile uint32_t g_sys_ticks_ms = 0; // make static to hide it from user as altering it can broke scheduler
static uint32_t idle_cnt = 0;
float cpu_load = 0; // number of SchedulerRun() executions in one tick of scheduler basic timing interrupt.
volatile uint16_t cpu_load_tmp = 0;

void SysTimerTask(void)
{
    // update system ticks
    g_sys_ticks_ms++;

    // clear idle cycles counter
    idle_cnt = 0;
}

uint32_t SchedulerGetTicks(void)
{
    return g_sys_ticks_ms;
}

void SchedulerRun(void)
{
  uint16_t i;

  // count idle cycles
  idle_cnt++;

  for (i=0;i<task_num;i++)
  {
    // is valid pointer ? and is time to run code ?
    if (
            (task_array[i].task_fptr != NULL) &&
            (task_array[i].is_running == 0) &&
            ((g_sys_ticks_ms >= (task_array[i].timestamp + task_array[i].period)) || (task_array[i].period == 0))
       )
    {
      task_array[i].timestamp = g_sys_ticks_ms; // set last run timestamp
      task_array[i].is_running = 1;
      task_array[i].task_fptr(); //run code
      task_array[i].is_running = 0;
      task_array[i].task_time = g_sys_ticks_ms - task_array[i].timestamp;
    }
  }
}

task_t* SchedulerAddTask(task_fnc_ptr_t fptr, uint32_t period)
{
    if (task_num < TASKS_MAX)
    {
        task_array[task_num].period = period;
        task_array[task_num].timestamp = 0;
        task_array[task_num].task_fptr = fptr;
        task_array[task_num].task_time = 0;
        task_array[task_num].is_running = 0;
#if DEBUG_TIMING == 1
        task_array[task_num].ts_delta = 0;
#endif
        return &task_array[task_num++];
    }
    return NULL;
}

void
SchedulerWaitMs(uint16_t wait_ms)
{
  uint32_t start_time = g_sys_ticks_ms;

  while((g_sys_ticks_ms - start_time) < wait_ms)
  {
    SchedulerRun(); // execute other tasks while waiting
  }
}


void SchedulerInit(float tick_period)
{
    uint16_t i;
    // disable all tasks in array
    for (i=0;i<TASKS_MAX;i++)
    {
        task_array[i].task_fptr = NULL;
    }
    task_num = 0;

    //
    // Setup SysTick - 100 us
    //
    MAP_SysTickPeriodSet(MAP_SysCtlClockGet()*tick_period);
    MAP_SysTickEnable();
    SysTickIntRegister(SysTimerTask);
    MAP_SysTickIntEnable();
}

/* EOF */
