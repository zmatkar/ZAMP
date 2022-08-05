//************************************
//*
//* ZAMP - ZmaTeK's amplifier
//*
//* T.Kosan 2022
//*
//************************************

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
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
#include "driverlib/adc.h"
#include "pinout.h"
#include "src/scheduler.h"
#include "display/gui.h"
#include "src/sigma_lib.h"
#include <src/user_eeprom.h>
#include "src/timing.h"
#include "display/lph7366_buffered.h"
#include "display/led_control.h"
#include "src/user_adc.h"
#include "src/user_input.h"
#include "src/relay_ctrl.h"

#define SCHEDULER_TICK (1e-3)

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
    while(1);
}
#endif

//*****************************************************************************
//
// Blink the on-board LED.
//
//*****************************************************************************
int
main(void)
{
    //
    // Enable stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    MAP_FPUEnable();
    MAP_FPUStackingEnable();

    //
    // Setup the system clock to run at 80 MHz from PLL with crystal reference
    //
    MAP_SysCtlClockSet(SYSCTL_SYSDIV_2_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|
                    SYSCTL_OSC_MAIN);

    //
    // Setup GPIO
    //
    PinoutSet();
    // early hardware mute
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, 0);

    TimingInit();

    SchedulerInit(SCHEDULER_TICK);
    MAP_IntMasterEnable();

    SigmaResetEnter();
    LedInit();
    UserInit();
    LcdInit();

    LcdPrintf("Booting...", 12, 24, 1, 0);
    LcdUpdate();

    SigmaResetRelease();

    // init ADAU1701
    SigmaInit();

    GuiInit();

    AdcInit();


    //
    // Loop forever.
    //
    while(1)
    {
        SchedulerRun();
    }
}
