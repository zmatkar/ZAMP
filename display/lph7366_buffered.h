/*
 * lph7366_buffered.h
 *
 *  Created on: 10. 3. 2021
 *      Author: zmatkar
 */

#ifndef DISPLAY_LPH7366_BUFFERED_H_
#define DISPLAY_LPH7366_BUFFERED_H_

#define MAX_Y 48
#define MAX_X 84

#define LCD_CONTRAST_MAX (0x3F)
#define LCD_CONTRAST_MIN (0x1F)
#define LCD_CONTRAST     (0x2F)

#define LCD_CLEAR_QUICK()     memset(frame_buffer, 0x0, sizeof(frame_buffer))

extern uint8_t frame_buffer[MAX_X][MAX_Y];

void LcdUpdate(void);

void LcdInit(void);

void LcdSetFgColor(uint8_t c);
uint8_t LcdGetFgColor(void);
void LcdClear(uint8_t data);
void LcdSetContrast(uint8_t data);
void LcdDrawRect(int8_t x, int8_t y, int8_t w, int8_t h);
void LcdDrawCircle(int8_t x0, int8_t y0, int8_t r);
void LcdDrawRoundRect(int8_t x, int8_t y, int8_t w, int8_t h, int8_t r);
void LcdFillCircle(int8_t x0, int8_t y0, int8_t r);
void LcdDrawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h);
void LcdDrawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1);
void LcdFillRect(int8_t x, int8_t y, uint8_t w, uint8_t h);
void LcdShowChar(uint8_t c, uint8_t x, uint8_t y, uint8_t scale, uint8_t invert);
void LcdPrintf(uint8_t *s, uint8_t x, uint8_t y, uint8_t scale, uint8_t invert);

#endif /* DISPLAY_LPH7366_BUFFERED_H_ */
