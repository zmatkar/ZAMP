/*
 * qei_ctrl.h
 *
 *  Created on: 12. 3. 2021
 *      Author: zmatkar
 */

#ifndef SRC_USER_INPUT_H_
#define SRC_USER_INPUT_H_

typedef enum{
    NO_PRESS,
    SHORT_PRESS,
    MID_PRESS,
    MID2_PRESS,
    LONG_PRESS
}switch_e;


extern int32_t velocity;
extern int32_t position;
extern switch_e switch_state;
extern switch_e switch_state_release;
extern uint16_t user_activity;
extern uint16_t press_activity;
extern float backlight;
/**
 * Init user interface, i.e. QEP peripheral.
 */
void UserInit(void);

/**
 * Acknowledge SW state.
 */
void UserAckSwState(void);

/**
 * Set slider to be controlled by QEP.
 */
void UserSetSlider(gui_slider_s* slider);

#endif /* SRC_USER_INPUT_H_ */
