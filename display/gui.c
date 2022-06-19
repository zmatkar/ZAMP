/*
 * gui.c
 *
 *  Created on: 24. 3. 2021
 *      Author: zmatkar
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "display/gui.h"
#include "src/scheduler.h"
#include "display/lph7366_buffered.h"
#include "display/icons.h"
#include "display/led_control.h"
#include "src/user_input.h"
#include "SigmaStudioFW.h"
#include "Amp_v3_IC_1_PARAM.h"
#include "src/sigma_lib.h"
#include <src/user_eeprom.h>
#include "src/timing.h"
#include "src/settings.h"
#include "driverlib/sysctl.h"
#include "src/relay_ctrl.h"

#define MAIN_DB_POSX 38
#define MAIN_DB_POSY 12

// position of input indicators
#define INPUT_POS_X 45
#define INPUT_POS_Y 0

#define EFF_POS_X 0
#define EFF_POS_Y 2

#define EQ_POS_X 20
#define EQ_POS_Y 43

#define MUTE_POS_X 76
#define MUTE_POS_Y 40

/*
 * LED support macros
 */
#define LED_NORMAL()          {sys_led.b=0; sys_led.r=0.0; sys_led.g=100.0; sys_led.i=led_intensity.value;}
#define LED_SCREENSAVER()     {sys_led.b=0; sys_led.r=100.0; sys_led.g=0.0; sys_led.i=led_intensity.value*0.5;}
#define LED_STANDBY()         {sys_led.b=0; sys_led.r=100.0; sys_led.g=0.0; sys_led.i=led_intensity.value*0.1;}

rgb_led_s sys_led;
static uint8_t buffer[256]; // buffer for string operations
gui_states_e gui_state = STARTUP;
float peak_value = 0.0;
float mid_value = 0.0;
uint16_t chan_select = 0;
uint32_t sampling_freq = 0;
float input_levels[5] = {-80.0, -80.0, -80.0, -80.0, -80.0};

/*
 * Settings sliders.
 */

gui_slider_s contrast_slider =
{
  .min = LCD_CONTRAST_MIN,
  .max = LCD_CONTRAST_MAX,
  .step = 1,
  .value = LCD_CONTRAST,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .show_as_db = 0,
  .enabled = 1
};

gui_slider_s backlight_slider =
{
  .min = 0,
  .max = 100.0,
  .step = 1.0,
  .value = 50.0,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 0
};

gui_slider_s led_intensity =
{
  .min = 0,
  .max = 100.0,
  .step = 1,
  .value = 50,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 0
};

gui_slider_s bloff_timeout =
{
  .min = 0,
  .max = 60.0,
  .step = 1,
  .value = 60,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = "Backlight off:",
  .enabled = 1,
  .show_as_db = 0
};

gui_slider_s noinp_timeout =
{
  .min = 0,
  .max = 60.0,
  .step = 1,
  .value = 60,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = "No-signal off:",
  .enabled = 1,
  .show_as_db = 0
};

/*
 * ADAU control sliders.
 */

gui_slider_s main_volume =
{
  .min = -80,
  .max = 16,
  .step = 1,
  .value = -80,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = "VOL",
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s balance =
{
  .min = -9,
  .max = 9,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = "BAL",
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s repro_gain =
{
  .min = -40,
  .max = 10,
  .step = 2,
  .value = -40,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s super_bass =
{
  .min = 0,
  .max = 1,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 0
};

gui_slider_s dynamic_bass =
{
  .min = 0,
  .max = 1,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 0
};

gui_slider_s phat_stereo =
{
  .min = -80.0,
  .max = 0,
  .step = 5,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s super_phat =
{
  .min = 0,
  .max = 1,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 0
};

gui_slider_s equalizer_enable =
{
  .min = 0,
  .max = 1,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 0
};

gui_slider_s eq_stage_0 =
{
  .min = -15,
  .max = 15,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s eq_stage_1 =
{
  .min = -15,
  .max = 15,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s eq_stage_2 =
{
  .min = -15,
  .max = 15,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s eq_stage_3 =
{
  .min = -15,
  .max = 15,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s eq_stage_4 =
{
  .min = -15,
  .max = 15,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s eq_stage_5 =
{
  .min = -15,
  .max = 15,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s eq_stage_6 =
{
  .min = -15,
  .max = 15,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s eq_stage_7 =
{
  .min = -15,
  .max = 15,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s eq_stage_8 =
{
  .min = -15,
  .max = 15,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 0,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s input_1 =
{
  .min = -30,
  .max = 12,
  .step = 1,
  .value = 0,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s input_2 =
{
  .min = -30,
  .max = 12,
  .step = 1,
  .value = -30,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s input_3 =
{
  .min = -30,
  .max = 12,
  .step = 1,
  .value = -30,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s input_4 =
{
  .min = -30,
  .max = 12,
  .step = 1,
  .value = -30,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s input_5 =
{
  .min = -30,
  .max = 12,
  .step = 1,
  .value = -30,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .show_as_db = 1
};

gui_slider_s mux_input_1 =
{
  .min = -30,
  .max = 12,
  .step = 1,
  .value = -30,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 1,
  .help_str = "INPUT A gain",
  .show_as_db = 1
};

gui_slider_s mux_input_2 =
{
  .min = -30,
  .max = 12,
  .step = 1,
  .value = -30,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 0,
  .help_str = "INPUT B gain",
  .show_as_db = 1
};

gui_slider_s mux_input_3 =
{
  .min = -30,
  .max = 12,
  .step = 1,
  .value = -30,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 0,
  .help_str = "INPUT C gain",
  .show_as_db = 1
};

gui_slider_s mux_input_4 =
{
  .min = -30,
  .max = 12,
  .step = 1,
  .value = -30,
  .can_overlap = 0,
  .mute_on_min = 1,
  .help_str = NULL,
  .enabled = 0,
  .help_str = "INPUT D gain",
  .show_as_db = 1
};

// 5x effects + 9x equalizer + 2x volume mute + 5x input selection
gui_slider_s* feat_controls[FEAT_COUNT] =
{
    &super_bass,
    &dynamic_bass,
    &phat_stereo,
    &equalizer_enable,
    &super_phat,
    &eq_stage_0,
    &eq_stage_1,
    &eq_stage_2,
    &eq_stage_3,
    &eq_stage_4,
    &eq_stage_5,
    &eq_stage_6,
    &eq_stage_7,
    &eq_stage_8,
    &repro_gain,
    &balance,
    &input_1, // analog in
    &input_2, // USB I2S
    &input_3,
    &input_4,
    &input_5,
    &main_volume
};

gui_slider_s* mux_inputs[MUXED_ANALOG_INPUTS] =
{
     &mux_input_1,
     &mux_input_2,
     &mux_input_3,
     &mux_input_4
};

uint8_t ui_setup_index = 0;
gui_slider_s* ui_setup[UI_SETUP_COUNT] =
{
     &contrast_slider,
     &backlight_slider,
     &led_intensity,
     &bloff_timeout,
     &noinp_timeout
};

float test_freq = 1000;
float test_freq_prev = 1000;
int32_t sigma_level_data[8];
int32_t sigma_level_data_max[8];
float sigma_level_db[8];
float sigma_level_db_max[8];
uint16_t about_pos = 0;


#define IDX_SB    (0)
#define IDX_DB    (1)
#define IDX_PH    (2)
#define IDX_EQ_EN (3)
#define IDX_SP    (4)
#define IDX_EQS0  (5)
#define IDX_EQS1  (6)
#define IDX_EQS2  (7)
#define IDX_EQS3  (8)
#define IDX_EQS4  (9)
#define IDX_EQS5  (10)
#define IDX_EQS6  (11)
#define IDX_EQS7  (12)
#define IDX_EQS8  (13)
#define IDX_REPRO (14)
#define IDX_BALANCE (15)
#define IDX_IN_1  (16)
#define IDX_IN_2  (17)
#define IDX_IN_3  (18)
#define IDX_IN_4  (19)
#define IDX_IN_5  (20)
#define IDX_MAIN_VOLUME  (21)

#define DB_TO_LIN(db)  (powf(10.0, (db)*0.05))
#define LIN_TO_DB(lin) (20.0*log10f((float)lin/Q_23))

void GuiDrawText(uint8_t* text, uint16_t x, uint16_t y, uint16_t length, bool selected)
{
    LcdPrintf(text, x+1, y+1, 1, selected);
    if (selected){
        LcdDrawLine(x, y+1, x, y+9);
        LcdDrawLine(x, y, x+length*6+1, y);
    }
}

void GuiDrawChar(char c, uint16_t x, uint16_t y, bool selected)
{
    LcdShowChar(c, x+1, y+1, 1, selected);
    if (selected){
        LcdDrawLine(x, y+1, x, y+9);
        LcdDrawLine(x, y, x+6+1, y);
    }
}

static void GuiRenderEditArrow(uint16_t feat_pos, uint16_t blinking)
{
    static uint16_t is_on = 0;

    if (((blinking == 1) && (is_on > 0xF)) || (blinking == 0))
    {
        // base features/effects
        if (feat_pos < 5){
            LcdDrawBitmap(EFF_POS_X+13, EFF_POS_Y + 2 + 9*feat_pos, icon_pointer_8x8, 5, 8);
        }else
            // equalizer
            if (feat_pos < 14){
                LcdDrawBitmap(EFF_POS_X+14+6*(feat_pos-4), EFF_POS_Y + 41, icon_pointer_up_8x8, 5, 5);
        }else
            // repro and phones
            if (feat_pos < 16){
                LcdDrawBitmap(MUTE_POS_X-4, MUTE_POS_Y+1-8*(feat_pos-14), icon_pointer_right_8x8_2, 4, 8);
        }else
            if (feat_pos < FEAT_COUNT-1){
                LcdDrawBitmap(INPUT_POS_X+1+8*(feat_pos-16), INPUT_POS_Y + 8, icon_pointer_up_8x8, 5, 3);
            }
    }
    is_on++;
    is_on &= 0x1F;
}

/*
 * Set gain for selected input.
 */
static void GuiUpdateInputMixer(gui_slider_s* input){

    input_mix_e index = INVALID;

    if (input == &input_1){
        // if (p_inputs_setup[0] == INPUT_ANALOG_SHARED)
        index = CH_AIN0;
    }
    if (input == &input_2){
        index = CH_DIN0;
    }
    if (input == &input_3){
        index = CH_DIN1;
    }
    if (input == &input_4){
        index = CH_DIN2;
    }
    if (input == &input_5){
        index = CH_DIN3;
    }

    if (input->value != input->prev_value){
        input->prev_value = input->value;
        SigmaInputMixer(index, DB_TO_LIN(input->value));
        if (input->value == input->min){
            SigmaMuteInput(index, 1);
        }else{
            SigmaMuteInput(index, 0);
        }
    }
}

static void GuiUpdateInputMultiplex(uint16_t mux_index)
{
    //TODO: relay switching
    RelayCtrl_Switch(mux_index);

    SigmaInputMixer(CH_AIN0, DB_TO_LIN(mux_inputs[mux_index]->value));
    if (mux_inputs[mux_index]->value != mux_inputs[mux_index]->prev_value)
    {
        mux_inputs[mux_index]->prev_value = mux_inputs[mux_index]->value;
        if (mux_inputs[mux_index]->value == mux_inputs[mux_index]->min){
            SigmaMuteInput(CH_AIN0, 1);
        }else{
            SigmaMuteInput(CH_AIN0, 0);
        }
    }
}

/*
 * Criss-cross input icon.
 */
static void GuiCrossInput(uint8_t x, uint8_t y, uint8_t is_active){
    if (is_active){
        LcdSetFgColor(0);
    }
    LcdDrawLine(x, y+1, x+7, y+8);
    LcdDrawLine(x, y+7, x+7, y+0);
    LcdSetFgColor(1);
}

static void GuiRenderMainScreen(uint16_t feat_index){
    static float vol_lin_left  = 0, vol_lin_prev_right = 0;
    static float vol_lin_right = 0, vol_lin_prev_left = 0;
    static float repro_lin = 0, repro_lin_prev = 0;
    static uint16_t vol_write_delay;
    static uint16_t vol_skip_write = 1;
    uint16_t i;

    uint16_t pos_x = MAIN_DB_POSX, pos_y = MAIN_DB_POSY, scale = 2;

    // render screen
    // show control value
    if (feat_controls[feat_index]->help_str){
        snprintf((char*)buffer, 4, "% 3s", feat_controls[feat_index]->help_str);
        LcdPrintf(buffer, MAIN_DB_POSX - 4 * 5, MAIN_DB_POSY+6, 1, 0);
    }
    if (feat_index != IDX_MAIN_VOLUME){
        // L-R balance setup
        if (feat_index == IDX_BALANCE){
            snprintf((char*)buffer, 7, "% +2d/% +2d\0", -1*(int32_t)feat_controls[feat_index]->value, (int32_t)feat_controls[feat_index]->value);
            pos_x = MAIN_DB_POSX;
            pos_y = MAIN_DB_POSY + 6;
            scale = 1;
        }else if (feat_controls[feat_index]->show_as_db){
            snprintf((char*)buffer, 4, "% +3d\0", (int32_t)feat_controls[feat_index]->value);
        }else{
            snprintf((char*)buffer, 4, "N/A\0");
        }
    }else{
        // main volume selected
        snprintf((char*)buffer, 4, "% +3d\0", (int32_t)main_volume.value);
    }

    LcdPrintf((uint8_t*)buffer, pos_x, pos_y, scale, 0);
    LcdPrintf("dB", MAIN_DB_POSX+34, MAIN_DB_POSY+6, 1, 0);

    // show effects and inputs indicators
    GuiDrawText("SB", EFF_POS_X, EFF_POS_Y, 2, feat_controls[IDX_SB]->value);
    GuiDrawText("DB", EFF_POS_X, EFF_POS_Y+9, 2, feat_controls[IDX_DB]->value);
    GuiDrawText("PH", EFF_POS_X, EFF_POS_Y+18, 2, feat_controls[IDX_PH]->value + 80.0);
    GuiDrawText("EQ", EFF_POS_X, EFF_POS_Y+27, 2, feat_controls[IDX_EQ_EN]->value);
    GuiDrawText("SP", EFF_POS_X, EFF_POS_Y+36, 2, feat_controls[IDX_SP]->value);

    //GuiDrawChar('0', INPUT_POS_X-8, INPUT_POS_Y, 1);

    // input 1 is special, it can have muxed input A-D
    for(i=0;i<4;i++){
        uint8_t is_active = feat_controls[i]->value - feat_controls[i]->min;
        if (mux_inputs[i]->enabled == 1){
            GuiDrawChar('A'+i, INPUT_POS_X, INPUT_POS_Y, is_active);
            if (input_levels[0] < -60.0){
                GuiCrossInput(INPUT_POS_X, INPUT_POS_Y, is_active);
            }
        }
    }

    // check signal on inputs
    for (i=IDX_IN_1+1;i<IDX_IN_1+5;i++){
        uint8_t is_active = feat_controls[i]->value - feat_controls[i]->min;
        uint8_t real_idx = i-IDX_IN_1;
        //GuiDrawText("1",  INPUT_POS_X, INPUT_POS_Y, 1, is_active);
        GuiDrawChar('1'+real_idx, INPUT_POS_X+8*real_idx, INPUT_POS_Y, is_active);
        if (input_levels[real_idx] < -60.0){
            GuiCrossInput(INPUT_POS_X+8*real_idx, INPUT_POS_Y, is_active);
        }
    }

#if 0
    GuiDrawText("2",  INPUT_POS_X+8, INPUT_POS_Y, 1, feat_controls[IDX_IN_2]->value  - feat_controls[IDX_IN_2]->min);
    GuiDrawText("3",  INPUT_POS_X+16, INPUT_POS_Y, 1, feat_controls[IDX_IN_3]->value - feat_controls[IDX_IN_3]->min);
    GuiDrawText("4",  INPUT_POS_X+24, INPUT_POS_Y, 1, feat_controls[IDX_IN_4]->value - feat_controls[IDX_IN_4]->min);
    GuiDrawText("5",  INPUT_POS_X+32, INPUT_POS_Y, 1, feat_controls[IDX_IN_5]->value - feat_controls[IDX_IN_5]->min);
#endif

    // repro and phone mute icons
    // TODO: on off ?
    LcdDrawBitmap(MUTE_POS_X, MUTE_POS_Y, icon_repro_8x8, 8, 8);
    LcdDrawBitmap(MUTE_POS_X, MUTE_POS_Y-9, icon_balance_8x8, 8, 8);

    // convert dB to linear
    vol_lin_left = DB_TO_LIN(main_volume.value - balance.value);
    vol_lin_right = DB_TO_LIN(main_volume.value + balance.value);
    repro_lin = DB_TO_LIN(repro_gain.value);

#if USE_VOL_AUTO_STORE == 1
    if ((vol_lin_right != vol_lin_prev_right) || (vol_lin_left != vol_lin_prev_left)){
        vol_write_delay = AUTOSAVE_VOL_S*1000/GUI_REFRESH_TIME_MS;
    }

    if (vol_write_delay > 0){
        vol_write_delay--;
        // write just once - when we reach zero
        if (vol_write_delay == 0){
            // skip first write
            if (vol_skip_write == 0){
                EepromWriteOne(&main_volume, IDX_MAIN_VOLUME);
            }
            vol_skip_write = 0;
        }
    }
#endif

    // apply only when changed
    if (vol_lin_left != vol_lin_prev_left){
        SigmaSetVolume(VOL_MAIN_L, vol_lin_left);
        vol_lin_prev_left = vol_lin_left;
    }
    if (vol_lin_right != vol_lin_prev_right){
        SigmaSetVolume(VOL_MAIN_R, vol_lin_right);
        vol_lin_prev_right = vol_lin_right;
    }

    if (repro_lin != repro_lin_prev){
        repro_lin_prev = repro_lin;
        SigmaSetVolume(VOL_REPRO_GAIN, repro_lin);
    }

    for(i=0;i<9;i++){
        gui_slider_s* p_eq_gain = feat_controls[i+IDX_EQS0];
        uint16_t height = (((int16_t)(p_eq_gain->value)+15)>>1)+1;
        // is EQ enabled ?
        if (feat_controls[IDX_EQ_EN]->value){
            // only if value changed
            if (p_eq_gain->prev_value != p_eq_gain->value){
                p_eq_gain->prev_value = p_eq_gain->value;
                SigmaSetEq(i, p_eq_gain->value);
                SchedulerWaitMs(1);
            }
            LcdFillRect(EQ_POS_X+i*6, EQ_POS_Y-height, 4, height);
        }else{
            LcdDrawRect(EQ_POS_X+i*6, EQ_POS_Y-height, 4, height);
        }
    }
    // EQ middle line - marks 0dB
    LcdDrawLine(EQ_POS_X-1, EQ_POS_Y-7, EQ_POS_X-1+9*6,  EQ_POS_Y-7);

    if (phat_stereo.prev_value != phat_stereo.value){
        SigmaSetPhatStereo(DB_TO_LIN(phat_stereo.value));
        phat_stereo.prev_value = phat_stereo.value;
        SchedulerWaitMs(1);
    }
    // apply changed flags
    if (super_bass.value != super_bass.prev_value){
        super_bass.prev_value = super_bass.value;
        SigmaSuperBassEnable(super_bass.value);
        SchedulerWaitMs(1);
    }
    if (dynamic_bass.value != dynamic_bass.prev_value){
        dynamic_bass.prev_value != dynamic_bass.value;
        SigmaEnableDynBass(dynamic_bass.value);
        SchedulerWaitMs(1);
    }
    if (super_phat.prev_value != super_phat.value){
        super_phat.prev_value = super_phat.value;
        SigmaEnableSPhat(super_phat.value);
        SchedulerWaitMs(1);
    }
    if (equalizer_enable.prev_value != equalizer_enable.value){
        equalizer_enable.prev_value = equalizer_enable.value;
        SigmaEqEnable(equalizer_enable.value);
        SchedulerWaitMs(1);
    }
    GuiUpdateInputMixer(&input_1);
    GuiUpdateInputMixer(&input_2);
    GuiUpdateInputMixer(&input_3);
    GuiUpdateInputMixer(&input_4);
    GuiUpdateInputMixer(&input_5);


#if 0
    if ((in_eff_flags.bits.TEST_TONE != in_eff_flags_prev.bits.TEST_TONE) ||
        (test_freq != test_freq_prev)){
        test_freq_prev = test_freq;
        in_eff_flags_prev.bits.TEST_TONE = in_eff_flags.bits.TEST_TONE;
        SigmaTestTone(test_freq, in_eff_flags.bits.TEST_TONE);
    }
#endif
    // show two bars of signals peaks
    const uint16_t xoff = 18;
    const uint16_t xwidth = 5;
    for(i=0;i<8;i++){

        int16_t peak = 0.25*(sigma_level_db[i]+47.0);
        int16_t peak_slow = 0.25*(sigma_level_db_max[i]+47.0);
        if (peak < 1){
            peak = 1;
        }
        if (peak > 15){
            peak = 15;
        }
        if (peak_slow < 1){
            peak_slow = 1;
        }
        if (peak_slow > 15){
            peak_slow = 15;
        }
        if (gui_state == MAINVOLUME){
            switch (i) {
                case 0:
                    LcdFillRect(xoff, 15-(peak), xwidth, (peak));
                    LcdDrawLine(xoff, 15-(peak_slow), xoff+xwidth, 15-(peak_slow));
                    break;
                case 1:
                    LcdFillRect(xoff+1+xwidth, 15-(peak), xwidth, (peak));
                    LcdDrawLine(xoff+1+xwidth, 15-(peak_slow), xoff+xwidth+1+xwidth, 15-(peak_slow));
                    break;
#if 0
                case 2:
                    LcdFillRect(xoff+2+2*xwidth, 15-(peak), xwidth, (peak));
                    LcdDrawLine(xoff+2+2*xwidth, 15-(peak_slow), xoff+xwidth+2+2*xwidth, 15-(peak_slow));
                    break;
                case 3:
                    LcdFillRect(xoff+3+3*xwidth, 15-(peak), xwidth, (peak));
                    LcdDrawLine(xoff+3+3*xwidth, 15-(peak_slow), xoff+xwidth+3+3*xwidth, 15-(peak_slow));
                    break;
#endif
                default:
                    break;
            }
        }
    }

    uint16_t xpos_inlev = xoff+2+2*xwidth;
    for(i=0;i<5;i++){
        int16_t peak = 0.25*(input_levels[i]+47.0);
        if (peak < 1){
            peak = 1;
        }
        if (peak > 15){
            peak = 15;
        }
        LcdDrawLine(xpos_inlev+i*2, 15-peak, xpos_inlev+i*2, 15);
    }
}

/**
 * Force to reconfigure Sigma/Adau parameters.
 */
static void GuiReconfigure(uint16_t force){
    uint16_t i;
    for(i=0;i<FEAT_COUNT;i++){
        feat_controls[i]->prev_value = feat_controls[i]->prev_value - feat_controls[i]->step;
    }
}

/**
 * Helper function to show name and state.
 * @param label Label to show
 * @param slider Pointer to slider instance with data to show.
 * @param y_pos Y postion on screen.
 */
static void GuiShowProgress(char* label, gui_slider_s* slider, uint8_t y_pos, bool active)
{
    LcdPrintf((uint8_t*)label, 0, y_pos, 1, 0);
    LcdDrawRect(0, y_pos+8, 83, 6);
    if (active){
        LcdFillRect(2, y_pos+10, 79*(slider->value-slider->min)/(slider->max-slider->min), 3);
    }else{
        LcdDrawRect(2, y_pos+10, 79*(slider->value-slider->min)/(slider->max-slider->min), 2);
    }
}

timing_t refresh_timing; // timing debug
uint8_t tries_to_reboot = 100; // if i2c fails continuously then reset
//uint8_t contrast = LCD_CONTRAST;

void GuiRefresh(void)
{
    uint16_t i;
    static uint16_t startup_cycles = 0;
    static uint32_t idle_timer = 0;
    static uint32_t user_idle_timer = 0;
    static float ss_x = 0, ss_y = 0;
    static float ss_xdir = 0.2, ss_ydir = 0.2;
    static int16_t feat_pos = 0;

    TimingStart(&refresh_timing);

    // clear whole screen
    LCD_CLEAR_QUICK();

    // show i2c error, it is set to 1000
    if (i2c_error > 0){
        SigmaHwMute(1);
        i2c_error--;
        LcdPrintf("I2C error !", 15, 16, 1, 0);
        LcdPrintf("Retrying ...", 15, 25, 1, 0);
        tries_to_reboot--;
        if (tries_to_reboot == 0){
            SysCtlReset();
        }
        LcdUpdate();
        SigmaResetEnter();
        SigmaResetRelease();
        SigmaInit();
        GuiReconfigure(1);
        return;
    }

    // check for Fs - required to proper function of ADAU
    if ((SigmaGetFs() != 48000) && (gui_state != STARTUP)){
        gui_state = STARTUP;
        startup_cycles = 0;
    }

    switch (gui_state) {
        case STARTUP:
            // show welcome screen
            LcdPrintf(WELCOME_1, 0, 0, 1, 0);
            LcdPrintf(WELCOME_2, 0, 8, 1, 0);
            LcdDrawRect(4, 16, 76, 8);
            LcdFillRect(6, 16+2, startup_cycles, 5);
            LcdPrintf("  Long press  ", 0, 25, 1, 0);
            LcdPrintf("four BL blinks", 0, 33, 1, 0);
            LcdPrintf(" to show help ", 0, 41, 1, 0);
            LcdUpdate();

            backlight = 0.9*backlight + 0.1*backlight_slider.value;

            //SchedulerWaitMs(5);
            startup_cycles++;
            if (startup_cycles == 72)
            {
                while (SigmaGetFs() != 48000){
                    uint8_t buffer[15];
                    uint16_t x = 0, y = 0;
                    LCD_CLEAR_QUICK();
                    LcdPrintf("Fs check...", x, y, 1, 0);
                    snprintf((char*)buffer, 14, "Fs=% 6d Hz\0", SigmaGetFs());
                    LcdPrintf(buffer, x, y+9, 1, 0);
                    LcdPrintf("Please set Fs", x, y+18, 1, 0);
                    LcdPrintf("to 48000 !!!!", x, y+27, 1, 0);
                    LcdUpdate();
                }
                gui_state = MAINVOLUME;
                UserSetSlider(&main_volume);
                SigmaHwMute(0);
            }
            break;

        case MAINVOLUME:
           LED_NORMAL();
           GuiRenderMainScreen(IDX_MAIN_VOLUME);
           // idle timer handling
           idle_timer++;
           // check for input signal
           if((input_levels[0] > INPUT_ACT_LEVEL) ||
              (input_levels[1] > INPUT_ACT_LEVEL) ||
              (input_levels[2] > INPUT_ACT_LEVEL) ||
              (input_levels[3] > INPUT_ACT_LEVEL) ||
              (input_levels[4] > INPUT_ACT_LEVEL))
           {
               idle_timer = 0;
               //backlight = backlight_slider.value;
           }
           if (user_activity > 0)
           {
               user_activity = 0;
               user_idle_timer = 0;
               backlight = backlight_slider.value;
           }

           if ((user_idle_timer > (1000*bloff_timeout.value)/GUI_REFRESH_TIME_MS) &&
               (bloff_timeout.value > 0.9))
           {
               backlight *= 0.9;
           }else{
               user_idle_timer++;
           }

           if ((idle_timer > (1000*noinp_timeout.value)/GUI_REFRESH_TIME_MS) &&
               (noinp_timeout.value > 0.9))
           {
               gui_state = SCREENSAVER;
               //backlight = backlight_slider.value;
               sys_led.i = led_intensity.value;
           }

           // button press handler
           if (switch_state_release == LONG_PRESS){
              gui_state = ABOUT;
              UserSetSlider(NULL);
              UserAckSwState();
           }
           if (switch_state_release == MID_PRESS){
              gui_state = FEAT_SETUP;
              UserAckSwState();
              feat_pos = 0;
           }
           if (switch_state_release == MID2_PRESS){
              gui_state = UI_SETUP;
              UserSetSlider(&contrast_slider);
              UserAckSwState();
              feat_pos = 0;
           }
           if (switch_state_release == SHORT_PRESS){
              gui_state = MUX_INPUT_SWITCH;
              UserAckSwState();
              // prepare selected input to be shown
              if (mux_input_1.enabled) feat_pos = 0;
              if (mux_input_2.enabled) feat_pos = 1;
              if (mux_input_3.enabled) feat_pos = 2;
              if (mux_input_4.enabled) feat_pos = 3;
           }
           break;

        case MUX_INPUT_SWITCH:
            UserSetSlider(NULL);
            GuiDrawText("AIN mux setup ", 0, 0, 14, 1);
            if ((velocity >> 2) > 0){
                feat_pos--;
                feat_pos &= 0x3; // range 0-3
                while(velocity != 0) SchedulerRun(); //wait for stop truning
                GuiUpdateInputMultiplex(feat_pos);
            }
            if ((velocity >> 2) < 0){
                feat_pos++;
                feat_pos &= 0x3; // range 0-3
                while(velocity != 0) SchedulerRun();
                GuiUpdateInputMultiplex(feat_pos);
            }

            GuiDrawText("IN A:", 0, 10, 5, feat_pos==0?1:0);
            GuiDrawText("IN B:", 0, 19, 5, feat_pos==1?1:0);
            GuiDrawText("IN C:", 0, 28, 5, feat_pos==2?1:0);
            GuiDrawText("IN D:", 0, 37, 5, feat_pos==3?1:0);

            {
                uint16_t i;
                for(i=0;i<4;i++){
                    mux_inputs[i]->enabled = feat_pos==i?1:0;
                    snprintf((char*)buffer, 14, "Au=% 2ddB\0", (int32_t)mux_inputs[i]->value);
                    LcdPrintf(buffer, 33, (i+1)*9+2, 1, 0);
                }
            }
            if (switch_state_release == SHORT_PRESS){
                UserAckSwState();
                gui_state = MUX_INPUT_SETUP;
                UserSetSlider(mux_inputs[feat_pos]);
            }

            if (switch_state_release == MID_PRESS){
                UserAckSwState();
                gui_state = MAINVOLUME;
                UserSetSlider(&main_volume);
                feat_pos = 0;
                EepromWriteInMux(mux_inputs, MUXED_ANALOG_INPUTS);
            }
            break;
        case MUX_INPUT_SETUP:
            GuiDrawText((uint8_t*)mux_inputs[feat_pos]->help_str, 5, 0, 12, 1);
            snprintf((char*)buffer, 14, "Au=% 2d dB\0", (int32_t)mux_inputs[feat_pos]->value);
            LcdPrintf(buffer, 16, 20, 1, 0);

            GuiUpdateInputMultiplex(feat_pos);

            if (switch_state_release == MID_PRESS){
                UserAckSwState();
                gui_state = MUX_INPUT_SWITCH;
                UserSetSlider(NULL);
            }
            break;

        case UI_SETUP:
            if (switch_state_release == SHORT_PRESS){
                if (ui_setup_index < (UI_SETUP_COUNT-1)){
                    ui_setup_index++;
                }else{
                    ui_setup_index = 0;
                }
                UserSetSlider(ui_setup[ui_setup_index]);
                UserAckSwState();
            }

            // show first page
            if (ui_setup_index < 3)
            {
                GuiShowProgress("Contrast:", ui_setup[0], 0, ui_setup_index == 0 ? 1:0);
                GuiShowProgress("Backlight:", ui_setup[1], 16, ui_setup_index == 1 ? 1:0);
                GuiShowProgress("LED intensity:", ui_setup[2], 32, ui_setup_index == 2 ? 1:0);
            }else{
//                GuiShowProgress("BLOFF timeout:", ui_setup[3], 0, ui_setup_index == 3 ? 1:0);
//                GuiShowProgress("NOINP timeout:", ui_setup[4], 24, ui_setup_index == 4 ? 1:0);
                {
                    uint16_t i;
                    for(i=0;i<2;i++){
                        LcdPrintf(ui_setup[i+3]->help_str, 0, i*18+2, 1, ui_setup_index == i+3 ? 1:0);
                        snprintf((char*)buffer, 14, "Timeout=% 2d s\0", (int32_t)ui_setup[i+3]->value);
                        LcdPrintf(buffer, 0, i*18+2+9, 1, 0);
                    }
                }
            }

            sys_led.i = led_intensity.value;
            backlight = backlight_slider.value;

            // exit back to volume control
            if (switch_state_release == MID_PRESS){
                UserAckSwState();
                gui_state = MAINVOLUME;
                UserSetSlider(&main_volume);
                EepromWriteSetup(ui_setup, UI_SETUP_COUNT);
            }
            break;

        case ABOUT:
            LcdPrintf("-----HELP-----", 0, 0, 1, 0);
            switch (about_pos) {
                case 0:
                    LcdPrintf(WELCOME_2, 0, 8, 1, 0);
                    LcdPrintf("  Long press  ", 0, 16, 1, 0);
                    LcdPrintf("     dial     ", 0, 24, 1, 0);
                    LcdPrintf(" to exit help.", 0, 32, 1, 0);
                    LcdPrintf(" Turn to move.", 0, 40, 1, 0);
                    break;
                case 1:
                    LcdPrintf("Basic control:", 0, 8, 1, 0);
                    LcdPrintf("SHORT  - one", 0, 16, 1, 0);
                    LcdPrintf("MIDDLE - two", 0, 24, 1, 0);
                    LcdPrintf("MID2   - three", 0, 32, 1, 0);
                    LcdPrintf("LONG   - four", 0, 40, 1, 0);
                    break;
                case 2:
                    LcdPrintf("Main UI:  ", 0, 8, 1, 0);
                    LcdPrintf("SHORT= MUX", 0, 16, 1, 0);
                    LcdPrintf("MID  = SETUP", 0, 24, 1, 0);
                    LcdPrintf("MID2 = SYSTEM", 0, 32, 1, 0);
                    LcdPrintf("LONG = HELP", 0, 40, 1, 0);
                    break;
                case 3:
                    LcdPrintf("Setup UI:  ", 0, 8, 1, 0);
                    LcdPrintf("SHORT = change", 0, 16, 1, 0);
                    LcdPrintf("MID   = BACK", 0, 24, 1, 0);
                    LcdPrintf("DIAL  = MOVE", 0, 32, 1, 0);
                    LcdPrintf("", 0, 40, 1, 0);
                    break;
                case 4:
                    LcdPrintf("<- status LED ", 0, 8, 1, 0);
                    LcdPrintf("Green :normal ", 0, 16, 1, 0);
                    LcdPrintf("Red   :standby", 0, 24, 1, 0);
                    LcdPrintf("Wake  :signal", 0, 32, 1, 0);
                    LcdPrintf("or DIAL action", 0, 40, 1, 0);
                    break;
                case 5:
                    LcdPrintf("Analog input: ", 0, 8, 1, 0);
                    LcdPrintf("Up-to +6dB sig", 0, 16, 1, 0);
                    LcdPrintf("Mixed with USB", 0, 24, 1, 0);
                    LcdPrintf("Input no. 1 ", 0, 32, 1, 0);
                    LcdPrintf("", 0, 40, 1, 0);
                    break;
                case 6:
                    about_pos = 0;
                    break;
                case 0xFFFF:
                    about_pos = 5;
                    break;
                default:
                    break;
            }
            LcdUpdate();
            if ((velocity >> 2) > 0){
                about_pos--;
                velocity = 0;
            }
            if ((velocity >> 2) < 0){
                about_pos++;
                velocity = 0;
            }
            // exit to main menu
            if (switch_state == MID_PRESS){
                gui_state = MAINVOLUME;
                UserSetSlider(&main_volume);
                UserAckSwState();
            }
            break;

        case FEAT_CHANGE:
            GuiRenderMainScreen(feat_pos);
            GuiRenderEditArrow(feat_pos, 1);
            if (switch_state_release == SHORT_PRESS){
                UserSetSlider(NULL);
                gui_state = FEAT_SETUP;
                UserAckSwState();
            }
            // exit back to volume control
            if (switch_state_release == MID_PRESS){
                UserAckSwState();
                gui_state = MAINVOLUME;
                UserSetSlider(&main_volume);
                // write changes to eeprom
                EepromWriteConfig(feat_controls, FEAT_COUNT);
            }
            break;

        case FEAT_SETUP:
            UserSetSlider(NULL);
            GuiRenderMainScreen(feat_pos);
            if (switch_state_release == SHORT_PRESS){
                UserSetSlider(feat_controls[feat_pos]);
                gui_state = FEAT_CHANGE;
                UserAckSwState();
            }
            // exit back to volume control
            if (switch_state_release == MID_PRESS){
                UserAckSwState();
                gui_state = MAINVOLUME;
                UserSetSlider(&main_volume);
                EepromWriteConfig(feat_controls, FEAT_COUNT);
            }
            if ((velocity >> 1) > 0){
                feat_pos--;
                velocity = 0;
            }
            if ((velocity >> 1) < 0){
                feat_pos++;
                velocity = 0;
            }
            if (feat_pos > (FEAT_COUNT-1)){
                feat_pos = 0;
            }
            if (feat_pos < 0){
                feat_pos = (FEAT_COUNT-1);
            }
            GuiRenderEditArrow(feat_pos, 0);
            break;

        case SCREENSAVER:
        {
            float bl_fade = 0.999;
            float led_fade = 0.999;

            UserSetSlider(NULL);
            LcdPrintf("No", ss_x+12, ss_y, 2, 0);
            LcdPrintf("input", ss_x, ss_y+16, 2, 0);
            if (ss_x > (MAX_X - 55)){
                ss_xdir = -0.2;
            }
            if (ss_x < 0){
                ss_xdir = 0.2;
            }
            if (ss_y > (MAX_Y - 30)){
                ss_ydir = -0.2;
            }
            if (ss_y < 0){
                ss_ydir = 0.2;
            }
            ss_x += ss_xdir;
            ss_y += ss_ydir;

            backlight *= bl_fade;
            sys_led.i *= led_fade;
            sys_led.i += 0.05;

            LED_SCREENSAVER();

            // check input, if active then exit stand-by
            if ((user_activity > 0) ||
                (input_levels[0] > INPUT_ACT_LEVEL) ||
                (input_levels[1] > INPUT_ACT_LEVEL) ||
                (input_levels[2] > INPUT_ACT_LEVEL) ||
                (input_levels[3] > INPUT_ACT_LEVEL) ||
                (input_levels[4] > INPUT_ACT_LEVEL))
            {
                idle_timer = 0;
                gui_state = MAINVOLUME;
                UserSetSlider(&main_volume);
                user_activity = 0;
                backlight = backlight_slider.value;
                sys_led.g = 100.0;
                sys_led.r = 0.0;
                sys_led.b = 0.0;
                sys_led.i = led_intensity.value;
            }
        }
            break;
        default:
            gui_state = MAINVOLUME;
            UserSetSlider(&main_volume);
            break;
    }

    // read interface registers 0-4
    // used to get output levels and input levels
    uint8_t u8_data[4*5];
    uint16_t reg_address = 0x800;
    SIGMA_READ_REGISTER_BLOCK(SIGMA_ADDR_I2C, reg_address, sizeof(u8_data), u8_data);
    // swap bytes to get valid uint32_t
    for(i=0;i<sizeof(u8_data);i+=4){
        sigma_level_data[i>>2] = (u8_data[i] << 24) | (u8_data[i+1] << 16) | (u8_data[i+2] << 8) | u8_data[i+3];
        sigma_level_data_max[i>>2] = sigma_level_data[i>>2] > sigma_level_data_max[i>>2] ? sigma_level_data[i>>2]:sigma_level_data_max[i>>2]*0.95;
    }

    for(i=0;i<5;i++){
        sigma_level_db[i] = LIN_TO_DB(sigma_level_data[i]);
        sigma_level_db_max[i] = LIN_TO_DB(sigma_level_data_max[i]);
    }

    LcdSetContrast(contrast_slider.value);
    //backlight = backlight_slider.value;
    //sys_led.i = led_intensity.value;
    // set panel RGB LED
    LedSet(sys_led);
    // refresh LCD display
    LcdUpdate();

    TimingEnd(&refresh_timing);
}

// slow task to check if any input has signal
void GuiInputsChecker(void)
{
    input_levels[chan_select] = sigma_level_db[4];

    if (chan_select < 4){
        chan_select++;
    }else{
        chan_select = 0;
    }
    // continuously scan inputs
    SigmaSelectInputDetector((input_mix_e)chan_select);

    // debug
    sampling_freq = SigmaGetFs();
}

void GuiInit(void)
{
    EepromInit();

    // setup autorefresh of display
    SchedulerAddTask(GuiRefresh, GUI_REFRESH_TIME_MS);
    SchedulerAddTask(GuiInputsChecker, GUI_INPUT_REFRESH_MS);

    // force update upon start
    uint16_t i;
    for(i=0;i<FEAT_COUNT;i++){
        feat_controls[i]->prev_value = feat_controls[i]->value - feat_controls[i]->step;
    }
    // default settings
    eq_stage_0.value = 8;
    eq_stage_1.value = 6;
    eq_stage_2.value = 0;
    eq_stage_3.value = 0;
    eq_stage_4.value = 0;
    eq_stage_5.value = 3;
    eq_stage_6.value = 8;
    eq_stage_7.value = 12;
    eq_stage_8.value = 15;
    equalizer_enable.value = 1;
    phat_stereo.value = -40.0;
    dynamic_bass.value = 1;

    EepromReadConfig(feat_controls, FEAT_COUNT);
    EepromReadSetup(ui_setup, UI_SETUP_COUNT);
    EepromReadInMux(mux_inputs, MUXED_ANALOG_INPUTS);
    // set main control to volume
    UserSetSlider(&main_volume);
    for(i=0;i<4;i++){
        if (mux_inputs[i]->enabled){
            GuiUpdateInputMultiplex(i);
        }
    }
    GuiRefresh();
}


