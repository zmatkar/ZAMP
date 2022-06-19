/*
 * led_control.h
 *
 *  Created on: 11. 3. 2021
 *      Author: zmatkar
 */

#ifndef DISPLAY_LED_CONTROL_H_
#define DISPLAY_LED_CONTROL_H_

typedef struct {
    float r;
    float g;
    float b;
    float i;
}rgb_led_s;

void LedSetDirect(uint16_t r, uint16_t g, uint16_t b);

/**
 * Set percentual mix of colors and intensity of system LED.
 * @param led Structure with required colors mix and intensity.
 */
void LedSet(rgb_led_s led);

/**
 * Set backlight intensity - use percentual value.
 * @param bl Percentual value to set backlight to.
 */
void LedSetBacklight(float bl);

void LedInit(void);


#endif /* SRC_LED_CONTROL_H_ */
