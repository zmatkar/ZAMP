/*
 * settings.h
 *
 *  Created on: 24. 9. 2021
 *      Author: zmatkar
 */

#ifndef SRC_SETTINGS_H_
#define SRC_SETTINGS_H_

/**
 * Two lines of welcome string shown upon startup.
 */
#define WELCOME_1   "ZAMP    1.1.1 "
#define WELCOME_2   "ZmaTKar, 19-22"

/**
 * GUI refresh period [ms].
 */
#define GUI_REFRESH_TIME_MS (50)
/**
 * Period of switching input signal levels detedtors.
 * Optimal value is 1s to ensure enough decay after switching.
 */
#define GUI_INPUT_REFRESH_MS (1000)

/**
 * Enable (1) auto saving main volume.
 */
#define USE_VOL_AUTO_STORE  (1)
/**
 * After time defined below the main volume is saved to EEPROM.
 * It has to be slightly lower value then idle time.
 */
#define AUTOSAVE_VOL_S    (15)
/**
 * Input signal level which resets idle and standby timer, causing the amplifier to wake up.
 */
#define INPUT_ACT_LEVEL   (-50.0)


#endif /* SRC_SETTINGS_H_ */
