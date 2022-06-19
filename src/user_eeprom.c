/*
 * eeprom.c
 *
 *  Created on: 21. 9. 2021
 *      Author: zmatkar
 */

#include <stdint.h>
#include <stdbool.h>
#include "display/gui.h"
#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"
#include "src/user_eeprom.h"

// start address
#define EE_UI_SETTINGS_START    (0x0)
// number of words (32bit) reserved for UI settings
// 4x16 blocks
#define EE_UI_SETTINGS_LEN      (256)

#if (FEAT_COUNT * 8) > (EE_UI_SETTINGS_LEN + EE_UI_SETTINGS_START)
#error Memory ranges of ADAU settings and system settings overlaps.
#endif

#define EE_SYS_SETTINGS_START   (EE_UI_SETTINGS_START+EE_UI_SETTINGS_LEN)
#define EE_SYS_SETTINGS_LEN     (256)

#define EE_INMUX_SETTINGS_START   (EE_SYS_SETTINGS_START+EE_SYS_SETTINGS_LEN)
#define EE_INMUX_SETTINGS_LEN     (256)

#define XOR_DATA_MASK   (0xFFAA55FF)

static     uint32_t ui32EEPROMInit;
union mixed_data{
    float flt_data;
    uint32_t u32_data;
};


void EepromWriteOne(gui_slider_s* slider, uint16_t index)
{
    uint32_t wrres = 0;
    union mixed_data data[2];
    // save if EEPROM works properly
    if (ui32EEPROMInit == EEPROM_INIT_OK)
    {
        data[0].flt_data = slider->value;
        data[1].u32_data = data[0].u32_data ^ XOR_DATA_MASK;
        wrres = EEPROMProgram((uint32_t*)data, index*8, 8);
    }
}

void EepromWriteConfig(gui_slider_s** sliders, uint16_t length)
{
    uint32_t wrres = 0;
    union mixed_data data[2];
    // save if EEPROM works properly
    if (ui32EEPROMInit == EEPROM_INIT_OK)
    {
        uint16_t i;
        for(i=0;i<length;i++){
            data[0].flt_data = sliders[i]->value;
            data[1].u32_data = data[0].u32_data ^ XOR_DATA_MASK;
            wrres = EEPROMProgram((uint32_t*)data, EE_UI_SETTINGS_START+i*8, 8);
        }
    }
}

void EepromReadConfig(gui_slider_s** sliders, uint16_t length)
{
    static uint16_t i;
    static union mixed_data data[2];
    // save if EEPROM works properly
    if (ui32EEPROMInit == EEPROM_INIT_OK)
    {
        for(i=0;i<length;i++){
            EEPROMRead((uint32_t*)data, EE_UI_SETTINGS_START+i*8, 8);
            // check data validity
            if (data[0].u32_data == (data[1].u32_data ^ XOR_DATA_MASK)){
                sliders[i]->value = data[0].flt_data;
            }else{
                sliders[i]->value = 0;
            }
        }
    }
}

void EepromWriteSetup(gui_slider_s** sliders, uint16_t length)
{
    uint32_t wrres = 0;
    union mixed_data data[2];
    // save if EEPROM works properly
    if (ui32EEPROMInit == EEPROM_INIT_OK)
    {
        uint16_t i;
        for(i=0;i<length;i++){
            data[0].flt_data = sliders[i]->value;
            data[1].u32_data = data[0].u32_data ^ XOR_DATA_MASK;
            wrres = EEPROMProgram((uint32_t*)data, EE_SYS_SETTINGS_START+i*8, 8);
        }
    }
}

void EepromReadSetup(gui_slider_s** sliders, uint16_t length)
{
    static uint16_t i;
    static union mixed_data data[2];
    // save if EEPROM works properly
    if (ui32EEPROMInit == EEPROM_INIT_OK)
    {
        for(i=0;i<length;i++){
            EEPROMRead((uint32_t*)data, EE_SYS_SETTINGS_START+i*8, 8);
            // check data validity
            if (data[0].u32_data == (data[1].u32_data ^ XOR_DATA_MASK)){
                sliders[i]->value = data[0].flt_data;
            }else{
                // set to half range as default
                sliders[i]->value = 0.5*(sliders[i]->max - sliders[i]->min) + sliders[i]->min;
            }
        }
    }
}

void EepromWriteInMux(gui_slider_s** sliders, uint16_t length)
{
    uint32_t wrres = 0;
    union mixed_data data[2];
    // save if EEPROM works properly
    if (ui32EEPROMInit == EEPROM_INIT_OK)
    {
        uint16_t i;
        for(i=0;i<length;i++){
            // store value as 16 bit integer, higher half word stores enable value
            data[0].u32_data = ((int16_t)sliders[i]->value & 0xFFFF) | ((uint32_t)sliders[i]->enabled << 16);
            data[1].u32_data = data[0].u32_data ^ XOR_DATA_MASK;
            wrres = EEPROMProgram((uint32_t*)data, EE_INMUX_SETTINGS_START+i*8, 8);
        }
    }
}

void EepromReadInMux(gui_slider_s** sliders, uint16_t length)
{
    static uint16_t i;
    static union mixed_data data[2];
    // save if EEPROM works properly
    if (ui32EEPROMInit == EEPROM_INIT_OK)
    {
        for(i=0;i<length;i++){
            EEPROMRead((uint32_t*)data, EE_INMUX_SETTINGS_START+i*8, 8);
            // check data validity
            if (data[0].u32_data == (data[1].u32_data ^ XOR_DATA_MASK)){
                sliders[i]->value = (int16_t)(data[0].u32_data & 0xFFFF);
                sliders[i]->enabled = data[0].u32_data >> 16;
            }else{
                // set to half range as default
                sliders[i]->value = 0.0;
                sliders[i]->enabled = i==0?1:0;
            }
        }
    }
}

void EepromInit(void)
{
    //
    // Enable the EEPROM module.
    //
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    //
    // Wait for the EEPROM module to be ready.
    //
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0)){}
    //
    // Wait for the EEPROM Initialization to complete
    //
    ui32EEPROMInit = EEPROMInit();

}

/* EOF */
