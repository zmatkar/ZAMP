/*
 * qei_ctrl.c
 *
 *  Created on: 12. 3. 2021
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
#include "driverlib/qei.h"
#include "driverlib/interrupt.h"
#include "pinout.h"
#include "src/scheduler.h"
#include "display/gui.h"
#include "user_input.h"
#include "display/led_control.h"

#define QEI_BASE QEI1_BASE

// 100ms process period
#define PROCESS_PERIOD  (100)

#define SHORT_PRESS_CYCLES  (5)
#define MID_PRESS_CYCLES    (10)
#define MID2_PRESS_CYCLES   (15)
#define LONG_PRESS_CYCLES   (20)

int32_t velocity = 0;
int32_t position = 0;

int32_t velocity_filt = 0;

float value = 0;
switch_e switch_state = NO_PRESS;
switch_e switch_state_release = NO_PRESS;
uint16_t user_activity = 0;
uint16_t press_activity = 0;

float backlight = 1.0;

static gui_slider_s* p_active_slider = NULL;

// this is safe offset to avoid overlap when user turn
// knob too quick
#define NO_OVERLAP_OFFSET (256)

void UserAckSwState(void){
    switch_state = NO_PRESS;
    switch_state_release = NO_PRESS;
}

void UserSetSlider(gui_slider_s* slider)
{
    p_active_slider = slider;

    if(slider){
        // avoid division by zero
        if (p_active_slider->step == 0.0){
            p_active_slider->step = 1.0;
        }
        uint16_t steps = ((p_active_slider->max - p_active_slider->min)/p_active_slider->step)*4;
        // configure QEI
        QEIConfigure(QEI_BASE, (QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_RESET | QEI_CONFIG_SWAP), steps + NO_OVERLAP_OFFSET*2);
        QEIPositionSet(QEI_BASE, NO_OVERLAP_OFFSET+((p_active_slider->value/p_active_slider->step) - p_active_slider->min)*4);
    }
}

gui_slider_s* UserGetSlider(void)
{
    return p_active_slider;
}

void UserProcessInputs(void)
{
    static int32_t push_state = 0;
    static uint32_t push_cnt = 0;
    static uint32_t blink_cycles = 5;

    // read QEI values
    position = QEIPositionGet(QEI_BASE);
    velocity = QEIVelocityGet(QEI_BASE) * QEIDirectionGet(QEI_BASE);

    // simple filter on velocity
    velocity_filt = 0.99 * velocity_filt + 0.01 * velocity;
    if (velocity != 0){
        user_activity++;
    }

    // do we have some active slider ?
    if (p_active_slider)
    {
        // number of steps
        uint16_t steps = ((p_active_slider->max - p_active_slider->min)/p_active_slider->step)*4;
        // real vlaue to show/store/use
        value = (0.25*(position - NO_OVERLAP_OFFSET)*p_active_slider->step + p_active_slider->min);
        // avoid over or under leaping
        if (position < NO_OVERLAP_OFFSET)
        {
            position = NO_OVERLAP_OFFSET;
            QEIPositionSet(QEI_BASE, position);
            value = p_active_slider->min;
        }

        if (position > (NO_OVERLAP_OFFSET + steps))
        {
            position = NO_OVERLAP_OFFSET + steps;
            QEIPositionSet(QEI_BASE, position);
            value = p_active_slider->max;
        }
        p_active_slider->value = value;
    }

    // handle push event
    int32_t tmp = GPIOPinRead(GPIO_PORTC_BASE, GPIO_PIN_7);

    if (push_cnt > LONG_PRESS_CYCLES)
    {
        switch_state = LONG_PRESS;
        press_activity = 2;
        push_cnt = 0;
    }

    // if 0 then pressed
    if (tmp == 0)
    {
        user_activity++;

        if (push_cnt > LONG_PRESS_CYCLES)
        {
            switch_state = LONG_PRESS;
            press_activity = 3;
        }
        else if (push_cnt > MID2_PRESS_CYCLES)
        {
            switch_state = MID2_PRESS;
            press_activity = 2;
        }
        else if (push_cnt > MID_PRESS_CYCLES)
        {
            switch_state = MID_PRESS;
            press_activity = 1;
        }
        else if (push_cnt > SHORT_PRESS_CYCLES)
        {
            switch_state = SHORT_PRESS;
        }
        if (push_cnt < 0xFFFFFFFF) push_cnt++; // avoid overflow
    }
    else
    {
        if (push_state == 0)
        {
            switch_state_release = switch_state;
        }
        switch_state = NO_PRESS;
        push_cnt = 0;
    }
    push_state = tmp;


    // ack press to user by LCD display blink
    if ((push_cnt == SHORT_PRESS_CYCLES) ||
        (push_cnt == MID_PRESS_CYCLES) ||
        (push_cnt == LONG_PRESS_CYCLES) ||
        (push_cnt == MID2_PRESS_CYCLES))
    {
        blink_cycles = 2; //200ms blink
    }

    // bink with backlight to inform user of
    if (blink_cycles > 0){
        blink_cycles--;
        LedSetBacklight(0.1*backlight);
    }else{
        LedSetBacklight(backlight);
    }
}

void UserInit(void)
{
    GPIOPadConfigSet(GPIO_PORTC_BASE, GPIO_PIN_5 | GPIO_PIN_6, GPIO_STRENGTH_8MA, GPIO_PIN_TYPE_STD);
    GPIODirModeSet(GPIO_PORTC_BASE, GPIO_PIN_6 | GPIO_PIN_5, GPIO_DIR_MODE_HW);

    //
    // Enable the QEI0 peripheral
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_QEI1);
    //
    // Wait for the QEI0 module to be ready.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_QEI1)){}
    //
    // Configure the quadrature encoder to capture edges on both signals and
    // maintain an absolute position by resetting on index pulses.  Using a
    // 1000 line encoder at four edges per line, there are 4000 pulses per
    // revolution; therefore set the maximum position to 3999 as the count
    // is zero based.
    //
    QEIConfigure(QEI_BASE, (QEI_CONFIG_CAPTURE_A_B | QEI_CONFIG_QUADRATURE | QEI_CONFIG_NO_RESET | QEI_CONFIG_SWAP), 256);

#if 1
    QEIFilterConfigure(QEI_BASE, QEI_FILTCNT_10);
    QEIFilterEnable(QEI_BASE);
#endif
    //
    // Enable the quadrature encoder.
    //
    QEIEnable(QEI_BASE);

    QEIVelocityConfigure(QEI_BASE, QEI_VELDIV_1, 8000000);
    QEIVelocityEnable(QEI_BASE);

    //
    // Register update function
    //
    SchedulerAddTask(UserProcessInputs, PROCESS_PERIOD);
}


