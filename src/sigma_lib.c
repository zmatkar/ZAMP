
/*
 * sigma_lib.c
 *
 *  Created on: 6. 8. 2021
 *      Author: zmatkar
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom_map.h"
#include "driverlib/i2c.h"
#include "adau/SigmaStudioFW.h"
#include "adau/Amp_v3_IC_1_PARAM.h"
#include "src/scheduler.h"
#include "src/sigma_lib.h"
#include "src/sigma_eq_params.h"

#define DO_I2C_UNSTUCK  (1)

// do hardware reset of DSP
void SigmaResetEnter(void)
{
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, 0);
    SchedulerWaitMs(1);
}

void SigmaResetRelease(void)
{
    GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_5, GPIO_PIN_5);
    SchedulerWaitMs(2); // wait for ADAU autoboot finish
}

void SigmaInit(void)
{

#if DO_I2C_UNSTUCK == 1
    MAP_GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_2); //SCL
    MAP_GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);
    MAP_GPIOPinTypeGPIOInput(GPIO_PORTB_BASE, GPIO_PIN_3); //SDA

    uint16_t cycles = 0;
    uint16_t value = GPIO_PIN_2;

    for(cycles=0;cycles<18;cycles++)
    {
        GPIOPinWrite(GPIO_PORTB_BASE, GPIO_PIN_2, value);
        value ^= 1;
        SysCtlDelay(30000);
        if (GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_3) == 0){
            break;
        }
    }
#endif

    //
    // Configure the GPIO Pin Mux for PB2
    // for I2C0SCL
    //
    MAP_GPIOPinConfigure(GPIO_PB2_I2C0SCL);
    MAP_GPIOPinTypeI2CSCL(GPIO_PORTB_BASE, GPIO_PIN_2);

    //
    // Configure the GPIO Pin Mux for PB3
    // for I2C0SDA
    //
    MAP_GPIOPinConfigure(GPIO_PB3_I2C0SDA);
    MAP_GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_3);

    //
    // Enable the I2C0 peripheral
    //
    SysCtlPeripheralEnable(SIGMA_I2C_PERIPH);
    //
    // Wait for the I2C0 module to be ready.
    //
    while(!SysCtlPeripheralReady(SIGMA_I2C_PERIPH)){}
    //
    // Initialize Master and Slave
    //
    I2CMasterInitExpClk(SIGMA_I2C, SysCtlClockGet(), true);

    // it is required to enable glitch filter - otherwise communication fails.
    I2CMasterGlitchFilterConfigSet(SIGMA_I2C, I2C_MASTER_GLITCH_FILTER_16);

    // load data
    SIGMA_LOAD_IC1();

    SigmaMuteInput(CH_AIN0, 1);
    SigmaMuteInput(CH_DIN0, 1);
    SigmaMuteInput(CH_DIN1, 1);
    SigmaMuteInput(CH_DIN2, 1);
    SigmaMuteInput(CH_DIN3, 1);
}

/**
 * Load register by using safe registers.
 * @param devAddress I2C address of device.
 * @param safeidx Index of safeload registers set, it can be in range <0,4>.
 * @param regAddr Address of register to write data to.
 * @param regData Data to write to address above.
 * @param apply If equals to 1 then safe write procedure is initialized
 */
static void SigmaSafeLoad(uint8_t devAddress, uint16_t safeidx, uint16_t regAddr, uint32_t regData, uint16_t apply)
{
    SIGMA_SAFELOAD_WRITE_ADDR(devAddress, SIGMA_SAFELOAD_ADDR_0 + safeidx, regAddr);
    SIGMA_SAFELOAD_WRITE_DATA(devAddress, SIGMA_SAFELOAD_DATA_0 + safeidx, regData);
    if (apply){
        SIGMA_SAFELOAD_WRITE_TRANSFER_BIT(devAddress);
    }
}

void SigmaTestTone(float freq, bool enable)
{
    uint32_t freqtmp = 0;
    freqtmp = MOD_INPUTS_TESTTONE_ALG0_INCREMENT_VALUE;// for 15000
    freqtmp = freq * (freqtmp/1000);
    SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_TESTTONE_ALG0_INCREMENT_ADDR, (uint32_t)freqtmp, 0);
    SigmaSafeLoad(SIGMA_ADDR_I2C, 1, MOD_INPUTS_TESTTONE_ALG0_ON_ADDR, (uint32_t)enable * Q_23, 1);
}

void SigmaSetVolume(volume_e volsel, float value)
{
    switch (volsel) {
        case VOL_REPRO_GAIN:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_OUTPUTS_REPROGAIN_ALG0_TARGET_ADDR, value * Q_23, 1);
            break;
        case VOL_MAIN_L:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_MAINVOLLEFT_ALG0_TARGET_ADDR, value * Q_23, 1);
            break;
        case VOL_MAIN_R:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_MAINVOLRIGHT_ALG0_TARGET_ADDR, value * Q_23, 1);
            break;
        default:
            break;
    }
}

void SigmaEnableDynBass(bool enable)
{
    SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_OUTPUTS_DYNBASS_OFF_ALG0_STAGE1_STEREOSWITCHNOSLEW_ADDR, (!enable) * Q_23, 0);
    SigmaSafeLoad(SIGMA_ADDR_I2C, 1, MOD_OUTPUTS_DYNBASS_OFF_ALG0_STAGE0_STEREOSWITCHNOSLEW_ADDR, enable * Q_23, 1);
}

void SigmaEnableSPhat(bool enable)
{
    SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_OUTPUTS_SPHAT_OFF_ALG0_STAGE0_STEREOSWITCHNOSLEW_ADDR, enable * Q_23, 0);
    SigmaSafeLoad(SIGMA_ADDR_I2C, 1, MOD_OUTPUTS_SPHAT_OFF_ALG0_STAGE1_STEREOSWITCHNOSLEW_ADDR, (!enable) * Q_23, 1);
}

void SigmaSuperBassEnable(bool enable)
{
    SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_OUTPUTS_SUPERBASS1_ALG0_SUPERBASSALGSWSLEW1BYPASS_ADDR, (!enable) * Q_23, 1);
}

void SigmaEqEnable(bool enable)
{
    SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_EQ_EQBYPASS_ALG0_STAGE0_STEREOSWITCHNOSLEW_ADDR, enable * Q_23, 0);
    SigmaSafeLoad(SIGMA_ADDR_I2C, 1, MOD_EQ_EQBYPASS_ALG0_STAGE1_STEREOSWITCHNOSLEW_ADDR, (!enable) * Q_23, 1);
}

void SigmaSetPhatStereo(float level)
{
    SigmaSafeLoad(SIGMA_ADDR_I2C, 0,
                  MOD_OUTPUTS_PHAT_STEREO1_ALG0_PHATSTEREOALG19401SPREADLEVEL_ADDR,
                  level * Q_23,
                  0);
    SigmaSafeLoad(SIGMA_ADDR_I2C, 1,
                  MOD_OUTPUTS_PHAT_STEREO1_ALG0_PHATSTEREOALG19401ALPHASPREAD_ADDR,
                  MOD_OUTPUTS_PHAT_STEREO1_ALG0_PHATSTEREOALG19401ALPHASPREAD_FIXPT,
                  1);
}

#if 0
void SigmaSetEq(uint16_t index, float boost)
{
    float B0 = 0.0, B1 = 0.0, B2 = 0.0, A1 = 0.0, A2 = 0.0, gainlinear = 1.0;
    float gain = 0.0;
    if (index < EQ_STAGES)
    {
        if (boost != 0.0)
        {
            float a0=0.0, omega=0.0, sn=0.0, cs=0.0, alpha=0.0, Ax=0.0;
            Ax = powf(10.0, boost*0.05);
            omega = 2*M_PI*eq_freq[index]/F_S;
            sn = sinf(omega);
            cs = cosf(omega);
            alpha = sn/(2*EQ_Q);

            a0 = 1 + (alpha/Ax);
            A1 = -(2*cs)/a0;
            A2 =  (1-(alpha/Ax))/a0;
            gainlinear = powf(10.0, (gain/20))/a0;

            B0 = (1+(alpha*Ax))*gainlinear;
            B1 = -(2*cs)*gainlinear;
            B2 = (1-(alpha*Ax))*gainlinear;
        }
    }
}
#else
void SigmaSetEq(uint16_t index, float boost)
{
    // boost index, it is from -15 to +15 including 0
    int16_t boost_index = (int16_t)boost + 15;

    // starting addresses of each satge
    const uint16_t eq_map[EQ_BAND_COUNT] =
    {
       MOD_EQ__ALG0_STAGE0_B0_ADDR,
       MOD_EQ__ALG0_STAGE1_B0_ADDR,
       MOD_EQ__ALG0_STAGE2_B0_ADDR,
       MOD_EQ__ALG0_STAGE3_B0_ADDR,
       MOD_EQ__ALG0_STAGE4_B0_ADDR,
       MOD_EQ__ALG0_STAGE5_B0_ADDR,
       MOD_EQ__ALG0_STAGE6_B0_ADDR,
       MOD_EQ__ALG0_STAGE7_B0_ADDR,
       MOD_EQ__ALG0_STAGE8_B0_ADDR
    };

    // avoid out-of-bound index
    if (((boost_index >= 0) && (boost_index < 31)) && (index < EQ_BAND_COUNT))
    {
        uint32_t i;
        // load five parameters
        for (i=0;i<5;i++){
            // load data from array
            int32_t data = eq_data[index][boost_index*5 + i];
            // swap bytes to get proper number
            data = ((data & 0xFF) << 24) | ((data & 0xFF00) << 8) | ((data & 0xFF0000) >> 8) | ((data & 0xFF000000) >> 24);
            if (i==4)
            {
                // if last data then start transfer
                SigmaSafeLoad(SIGMA_ADDR_I2C, i, eq_map[index]+i, data, 1);
            }else{
                SigmaSafeLoad(SIGMA_ADDR_I2C, i, eq_map[index]+i, data, 0);
            }
        }
    }
}
#endif

void SigmaMuteInput(input_mix_e ch, bool mute)
{
    switch (ch) {
        case CH_AIN0:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_MUTEDIN0_2_ALG0_MUTEONOFF_ADDR, !mute * Q_23, 1);
            break;
        case CH_DIN0:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_MUTEDIN0_ALG0_MUTEONOFF_ADDR, !mute * Q_23, 1);
            break;
        case CH_DIN1:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_MUTEDIN1_ALG0_MUTEONOFF_ADDR, !mute * Q_23, 1);
            break;
        case CH_DIN2:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_MUTEDIN2_ALG0_MUTEONOFF_ADDR, !mute * Q_23, 1);
            break;
        case CH_DIN3:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_MUTEDIN3_ALG0_MUTEONOFF_ADDR, !mute * Q_23, 1);
            break;
        case CH_TESTTONE:
            break;
        default:
            break;
    }
}

void SigmaInputMixer(input_mix_e ch, float gainDb)
{
    switch (ch) {
        case CH_AIN0:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_INPUTMIXER_ALG0_STAGE0_VOLUME_ADDR, gainDb * Q_23, 1);
            break;
        case CH_DIN0:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_INPUTMIXER_ALG0_STAGE1_VOLUME_ADDR, gainDb * Q_23, 1);
            break;
        case CH_DIN1:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_INPUTMIXER_ALG0_STAGE2_VOLUME_ADDR, gainDb * Q_23, 1);
            break;
        case CH_DIN2:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_INPUTMIXER_ALG0_STAGE3_VOLUME_ADDR, gainDb * Q_23, 1);
            break;
        case CH_DIN3:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUTS_INPUTMIXER_ALG0_STAGE4_VOLUME_ADDR, gainDb * Q_23, 1);
            break;
        case CH_TESTTONE:
            break;
        default:
            break;
    }
}

void SigmaSelectInputDetector(input_mix_e ch)
{
    switch (ch) {
        case CH_AIN0:

            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE0_MONOSWITCHNOSLEW_ADDR, 1 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 1, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE1_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 2, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE2_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 3, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE3_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 4, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE4_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 1);
            break;
        case CH_DIN0:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE0_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 1, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE1_MONOSWITCHNOSLEW_ADDR, 1 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 2, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE2_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 3, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE3_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 4, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE4_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 1);
            break;
        case CH_DIN1:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE0_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 1, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE1_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 2, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE2_MONOSWITCHNOSLEW_ADDR, 1 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 3, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE3_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 4, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE4_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 1);
            break;
        case CH_DIN2:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE0_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 1, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE1_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 2, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE2_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 3, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE3_MONOSWITCHNOSLEW_ADDR, 1 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 4, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE4_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 1);
            break;
        case CH_DIN3:
            SigmaSafeLoad(SIGMA_ADDR_I2C, 0, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE0_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 1, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE1_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 2, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE2_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 3, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE3_MONOSWITCHNOSLEW_ADDR, 0 * Q_23, 0);
            SigmaSafeLoad(SIGMA_ADDR_I2C, 4, MOD_INPUT_DETECTORS_SELECTOR_ALG0_STAGE4_MONOSWITCHNOSLEW_ADDR, 1 * Q_23, 1);
            break;
        case CH_TESTTONE:
            break;
        default:
            break;
    }
}

void SigmaHwMute(bool enable)
{
    GPIOPinWrite(GPIO_PORTD_BASE, GPIO_PIN_1, (enable == false)?GPIO_PIN_1:0);
}

uint32_t SigmaGetFs(void){
    const uint32_t fs_map[8] = {44100, 48000, 88200, 96000, 176400, 192000, 352800, 384000};
    uint8_t idx = 0;
    if(GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_3)){
        idx |= 1<<0;
    }
    if(GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2)){
        idx |= 1<<1;
    }
    if(GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_0)){
        idx |= 1<<2;
    }
    return fs_map[idx];
}

/* EOF */
