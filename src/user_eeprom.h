/*
 * eeprom.h
 *
 *  Created on: 21. 9. 2021
 *      Author: zmatkar
 */

#ifndef SRC_USER_EEPROM_H_
#define SRC_USER_EEPROM_H_

typedef enum{
    CONTRAST = 0,
    BACKLIGHT = 1,
    LED_INTENS = 2
}setup_item_e;

void EepromInit(void);

void EepromWriteOne(gui_slider_s* slider, uint16_t index);

void EepromWriteConfig(gui_slider_s** sliders, uint16_t length);

void EepromReadConfig(gui_slider_s** sliders, uint16_t length);

void EepromReadSetup(gui_slider_s** sliders, uint16_t length);

void EepromWriteSetup(gui_slider_s** sliders, uint16_t length);

void EepromReadInMux(gui_slider_s** sliders, uint16_t length);

void EepromWriteInMux(gui_slider_s** sliders, uint16_t length);

#endif /* SRC_USER_EEPROM_H_ */
