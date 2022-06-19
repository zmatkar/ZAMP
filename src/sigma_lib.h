/*
 * sigma_lib.h
 *
 *  Created on: 6. 8. 2021
 *      Author: zmatkar
 *
 *  Sigma FW wrapper.
 */

#ifndef SRC_SIGMA_LIB_H_
#define SRC_SIGMA_LIB_H_

// base number format Q5.23, int32_t
#define Q_23    (1L<<23)

// samplerate
#define F_S     (96000.0)

typedef enum{
    CH_AIN0 = 0,
    CH_DIN0 = 1,
    CH_DIN1 = 2,
    CH_DIN2 = 3,
    CH_DIN3 = 4,
    CH_TESTTONE = 5,
    INVALID = 6
}input_mix_e;

typedef enum{
    VOL_REPRO_GAIN,
    VOL_MAIN_L,
    VOL_MAIN_R
}volume_e;

/**
 * Do hardware reset.
 */
void SigmaResetRelease(void);
void SigmaResetEnter(void);

/**
 * Hardware mute independent on ADAU state.
 */
void SigmaHwMute(bool enable);

/**
 * Load firmware to Adau DSP.
 */
void SigmaInit(void);

/**
 * Control Mute blocks.
 * @param ch Use one of defined channels, see input_mix_e.
 * @param mute If 1 then input is muted, it is not muted when 0.
 */
void SigmaMuteInput(input_mix_e ch, bool mute);

/**
 * Control of input Stereo mixer.
 * @param ch Use one of defined channels, see input_mix_e.
 * @param gainDb Set gain of selected input (for both channels L+R).
 */
void SigmaInputMixer(input_mix_e ch, float gainDb);

/**
 * Sixth input is test tone.
 * @param freq Frequency of tone generator.
 * @param enable Turn generator on or off.
 */
void SigmaTestTone(float freq, bool enable);

/**
 * Set 7 range equalizer.
 * @param gains Pointer to structure which consist of gains for each EQ range gain [dB], <-10,+10>.
 */
void SigmaSetEq(uint16_t index, float boost);

/**
 * Enable/disable dynamic bass boost block.
 * @param enable Enable/disable block.
 */
void SigmaEnableDynBass(bool enable);

/**
 * Enable/disable SuperPhat Spatializer block.
 * @param enable Enable/disable block.
 */
void SigmaEnableSPhat(bool enable);

/**
 * Enable/disable super bass block.
 * @param enable
 */
void SigmaSuperBassEnable(bool enable);

/**
 * Enable/disable equalizer.
 */
void SigmaEqEnable(bool enable);

void SigmaSetPhatStereo(float level);

/**
 * Set output volume level.
 * @param volsel Can be one of volume_e items.
 * @param value Volume value to set in dB <-80,0>.
 */
void SigmaSetVolume(volume_e volsel, float value);

void SigmaSelectInputDetector(input_mix_e ch);

uint32_t SigmaGetFs(void);

#endif /* SRC_SIGMA_LIB_H_ */
