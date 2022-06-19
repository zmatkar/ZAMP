/*
 * gui.h
 *
 *  Created on: 24. 3. 2021
 *      Author: zmatkar
 */

#ifndef DISPLAY_GUI_H_
#define DISPLAY_GUI_H_

/**
 * Default inputs configuration.
 */
extern uint8_t inputs_setup_1[5];
/**
 * Shared analog input and no I2S inputs.
 * No mixing possible,
 */
extern uint8_t inputs_setup_2[5];

/*
 * Number of features control - i.e. equaliyer, volume, effects, etc.
 */
#define FEAT_COUNT  (22)
/*
 * Number of sliders for setup.
 */
#define UI_SETUP_COUNT (5)
/*
 *
 */
#define MUXED_ANALOG_INPUTS (4)

/**
 * This is basic control structure.
 * It is used to control all Sigma Studio
 * variables and some internal.
 */
typedef struct {
    float min;                  ///< Minimal value.
    float max;                  ///< Maximal value.
    float step;                 ///< One step increment.
    float value;                ///< Current value.
    float prev_value;           ///< Previous value - used to detect changes.
    uint16_t can_overlap;       ///< If set to 1 then value can overlap from min to max and back.
    uint16_t mute_on_min;       ///< Mute Sigma Studio control if minimum is reached. User must implement this.
    uint16_t show_as_db;        ///< If 1 then current value will be shown when this slider is active instead of main volume.
    uint16_t enabled;           ///< If 1 then slider is enabled. Used with analog muxed inputs.
    char* help_str;             ///< Help string, just 3 visible characters allowed.
}gui_slider_s;

/**
 * Test tone control structure. Can be used to beep to user.
 */
struct test_tone_s{
    uint32_t mute;  ///> If 1 then muted - default.
    uint32_t freq;  ///> Frequency of tone. It is dependent on sampling frequency.
    uint32_t gain;  ///> Gain of output - i.e. volume control.
};

/**
 * Current GUI item/slider structure.
 */
typedef struct{
    char* name;
    void (*callback_ptr)(void);
}menu_item_s;

/**
 * Enumerate od GUI implemented states.
 */
typedef enum{
    STARTUP,       //!< STARTUP - this do some checks during start also when Fs changed during run-time.
    MAINVOLUME,    //!< MAINVOLUME - main screen render.
    MENU,          //!< MENU - menu, currently not used.
    ABOUT,         //!< ABOUT - help + about
    UI_SETUP,      //!< UI_SETUP - contrast/backlight and LED intensity setup.
    SCREENSAVER,   //!< SCREENSAVER - if idle (no signal) for more than 10 minutes this state is activated.
    FEAT_SETUP,    //!< FEAT_SETUP - renders cursor which moves around all parameters.
    MUX_INPUT_SWITCH,  //!< INPUT_SWITCH - renders quick input switch screen.
    MUX_INPUT_SETUP,   //!< INPUT_SETUP - allows user to set gain for muxed analog input.
    FEAT_CHANGE,   //!< FEAT_CHANGE - implements value change.
}gui_states_e;

/**
 * Current GUI state.
 */
extern gui_states_e gui_state;

/**
 * Repaint/render GUI according
 * current state.
 */
void GuiRefresh(void);

/**
 * Init whole GUI subsystem.
 */
void GuiInit(void);

#endif /* DISPLAY_GUI_H_ */
