/*
 * lph7366_buffered.c
 *
 *  Created on: 10. 3. 2021
 *      Author: zmatkar
 */


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "inc/hw_memmap.h"
#include "driverlib/rom_map.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/timer.h"
#include "driverlib/ssi.h"
#include "driverlib/fpu.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "pinout.h"
#include "src/scheduler.h"
#include "display/lph7366_buffered.h"
#include "display/font_8x5.h"

#define LCD_PORT    (GPIO_PORTA_BASE)
#define LCD_D_C     (GPIO_PIN_0)
#define LCD_nRES    (GPIO_PIN_1)
#define SSI_PORT    (SSI0_BASE)

#define BIT_PD (1<<0)
#define BIT_V  (1<<1)
#define BIT_H  (1<<2)

#define BIT_D  (1<<2)
#define BIT_E  (1<<0)

#define DISP_MODE_BLANK     (0x08)
#define DISP_MODE_NORMAL    (0x08 | BIT_D)
#define DISP_MODE_ALLON     (0x08 | BIT_E)
#define DISP_MODE_INVERSE   (0x08 | BIT_D | BIT_E)


// *************************************************
// Display Mode
// 0x0C for normal, 0x0D for inverse
// *************************************************
#define DISPLAY_MODE DISP_MODE_NORMAL

#define COMMAND (0)
#define DATA (1)

#define SSI_FLUSH()    while(SSIBusy(SSI_PORT))

uint8_t frame_buffer[MAX_X][MAX_Y]; // one byte -> one pixel :(

static uint8_t color = 0x1;

static void LCDWriteFB(int8_t x, int8_t y, uint8_t data)
{
    while (x > 83) x -= 84;
    while (y > 47) y -= 48;
    while (x < 0)  x += 84;
    while (y < 0)  y += 48;
    frame_buffer[x][y] = data;
}

/**
 * Basic low- level SPI write function.
 * @param data  data to write.
 * @param command set to 1 for command or 0 for data transfer.
 */
static void LCDwrite(uint8_t data, uint8_t command)
{
    GPIOPinWrite(LCD_PORT, LCD_D_C, LCD_D_C * command);
    SSIDataPut(SSI_PORT, data);
    // block execution for command send, data can be filled to FIFO
    while((SSIBusy(SSI_PORT) == true) && (command == COMMAND));

    GPIOPinWrite(LCD_PORT, LCD_D_C, LCD_D_C);
}

/**
 * Set cursor in display buffer to exact position.
 * @param x_pos X axis position
 * @param y_pos Y axis position
 */
static void LCDSetCursor(uint8_t x_pos, uint8_t y_pos){
    if(x_pos > 83 || y_pos > 5) return; // bad inputs, do nothing and return
    LCDwrite(0x80|(x_pos & 0x7F), COMMAND);    // setting bit 7 updates X-position
    LCDwrite(0x40|(y_pos & 0x7), COMMAND);      // setting bit 6 updates Y-position
    SSI_FLUSH();
}

/**
 * Main update function.
 * Responsible for transfer data from internal buffer
 * to display. Call it to refresh pixels shown on display.
 */
void LcdUpdate(void)
{
    int8_t x,y;

    // start at begining
    LCDSetCursor(0, 0);

    // Y axis has to be swapped
    for (y=(MAX_Y-8);y>=0;y-=8)
    {
        // X has to be swapped
        for (x=(MAX_X-1);x>=0;x--)
        {
            uint16_t data = 0;
            // get 8 pixels to form one data byte
            data |=  (uint16_t)(frame_buffer[x][y+7] & 0x1)  << 0;
            data |= (frame_buffer[x][y+6] & 0x1) << 1;
            data |= (frame_buffer[x][y+5] & 0x1) << 2;
            data |= (frame_buffer[x][y+4] & 0x1) << 3;

            data |= (frame_buffer[x][y+3] & 0x1) << 4;
            data |= (frame_buffer[x][y+2] & 0x1) << 5;
            data |= (frame_buffer[x][y+1] & 0x1) << 6;
            data |= (uint16_t)(frame_buffer[x][y] & 0x1) << 7;
            // write data to display
            LCDwrite(data, DATA);
        }
    }
    // make all data to be written - avoid artifacts
    SSI_FLUSH();
}

/**
 * Write user defined data to display.
 * @param data Data to be written to whole display memory.
 */
void LCDClear(uint8_t data) {
    uint16_t i;
    LCDSetCursor(0, 0);
    for(i = 0; i < ((MAX_X*MAX_Y/8)); i++) {
        LCDwrite(data, DATA);
    }
    SSI_FLUSH();
    LCDSetCursor(0, 0);
}

/**
 * Set contrast of display
 * @param data Contrast to be set.
 */
void LcdSetContrast(uint8_t data)
{
    LCDwrite(0x21, COMMAND); // power up + hor. adress + ext. mode
    LCDwrite((data & 0x7F) | 0x80, COMMAND);
    LCDwrite(0x20, COMMAND);
    SSI_FLUSH();
}


/**
 * Swap operands to according to their value. Lower first.
 * @param op1
 * @param op2
 */
static void sort_int8(int8_t* op1, int8_t* op2)
{
    int8_t t;
    if (*op1 > *op2)
    {
        t = *op1;
        *op1 = *op2;
        *op2 = t;
    }
}

/**
 * Fast draw horizontal line.
 * @param x0 Start point X.
 * @param x1 End point X.
 * @param y Y position.
 */
static void FastHLine(int8_t x0, int8_t x1, int8_t y)
{
    sort_int8(&x0, &x1); // swap x0 and x1 if needed to always go up
    int8_t i;
    for(i=x0;i<x1;i++)
    {
        //frame_buffer[i][y] = color;
        LCDWriteFB(i, y, color);
    }
}

/**
 * Fast draw vertical line.
 * @param y0 Start position Y.
 * @param y1 End position Y.
 * @param x X position.
 */
static void FastVLine(int8_t y0, int8_t y1, int8_t x)
{
    sort_int8(&y0, &y1); // swap y0 and y1 if needed to always go up
    int8_t i;
    for(i=y0;i<y1;i++)
    {
        //frame_buffer[x][i] = color;
        LCDWriteFB(x, i, color);
    }
}

/**
 * Draw filled rectangle.
 * @param x Starting X position.
 * @param y Starting Y position.
 * @param w Width.
 * @param h Height.
 */
void LcdFillRect(int8_t x, int8_t y, uint8_t w, uint8_t h)
{
    int8_t i;
    for(i=y; i<y+h; i++)
    {
        FastHLine(x, x+w, i);
    }
}

void LcdDrawRect(int8_t x, int8_t y, int8_t w, int8_t h)
{
  FastHLine(x, x+w, y);
  FastHLine(x, x+w, y + h);
  FastVLine(y, y+h, x);
  FastVLine(y, y+h, x+w);
}

void LcdDrawCircle(int8_t x0, int8_t y0, int8_t r) {

  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * r;
  int8_t x = 0;
  int8_t y = r;

  //frame_buffer[x0][y0 + r] = color;
  LCDWriteFB(x0, y0 + r, color);
  //frame_buffer[x0][y0 - r] = color;
  LCDWriteFB(x0, y0 - r, color);
  //frame_buffer[x0 + r][y0] = color;
  LCDWriteFB(x0 + r, y0, color);
  //frame_buffer[x0 - r][y0] = color;
  LCDWriteFB(x0 - r, y0, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    //frame_buffer[x0 + x][y0 + y] = color;
    LCDWriteFB(x0 + x, y0 + y, color);
    //frame_buffer[x0 - x][y0 + y] = color;
    LCDWriteFB(x0 - x, y0 + y, color);
    //frame_buffer[x0 + x][y0 - y] = color;
    LCDWriteFB(x0 + x, y0 - y, color);
    //frame_buffer[x0 - x][y0 - y] = color;
    LCDWriteFB(x0 - x, y0 - y, color);
    //frame_buffer[x0 + y][y0 + x] = color;
    LCDWriteFB(x0 + y, y0 + x, color);
    //frame_buffer[x0 - y][y0 + x] = color;
    LCDWriteFB(x0 - y, y0 + x, color);
    //frame_buffer[x0 + y][y0 - x] = color;
    LCDWriteFB(x0 + y, y0 - x, color);
    //frame_buffer[x0 - y][y0 - x] = color;
    LCDWriteFB(x0 - y, y0 - x, color);
  }
}

/**************************************************************************/
/*!
    @brief  Quarter-circle drawer with fill, used for circles and roundrects
    @param  x0       Center-point x coordinate
    @param  y0       Center-point y coordinate
    @param  r        Radius of circle
    @param  corners  Mask bits indicating which quarters we're doing
    @param  delta    Offset from center-point, used for round-rects
    @param  color    16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
static void fillCircleHelper(int8_t x0, int8_t y0, int8_t r, uint8_t corners, int8_t delta) {

  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * r;
  int8_t x = 0;
  int8_t y = r;
  int8_t px = x;
  int8_t py = y;

  delta++; // Avoid some +1's in the loop

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    if (x < (y + 1)) {
      if (corners & 1)
        //FastVLine(x0 + x, y0 - y, 2 * y + delta);
          FastVLine(y0 - y, y0 - y + (2 * y + delta), x0 + x);
      if (corners & 2)
        FastVLine(y0 - y, y0 - y + (2 * y + delta), x0 - x);
    }
    if (y != py) {
      if (corners & 1)
        FastVLine(y0 - px, y0 - px + (2 * px + delta), x0 + py);
      if (corners & 2)
        FastVLine(y0 - px, y0 - px + (2 * px + delta), x0 - py);
      py = y;
    }
    px = x;
  }
}

static void LCDDrawCircleHelper(int8_t x0, int8_t y0, int8_t r, uint8_t cornername) {
  int8_t f = 1 - r;
  int8_t ddF_x = 1;
  int8_t ddF_y = -2 * r;
  int8_t x = 0;
  int8_t y = r;

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    if (cornername & 0x4) {
      LCDWriteFB(x0 + x, y0 + y, color);
      LCDWriteFB(x0 + y, y0 + x, color);
    }
    if (cornername & 0x2) {
      LCDWriteFB(x0 + x, y0 - y, color);
      LCDWriteFB(x0 + y, y0 - x, color);
    }
    if (cornername & 0x8) {
      LCDWriteFB(x0 - y, y0 + x, color);
      LCDWriteFB(x0 - x, y0 + y, color);
    }
    if (cornername & 0x1) {
      LCDWriteFB(x0 - y, y0 - x, color);
      LCDWriteFB(x0 - x, y0 - y, color);
    }
  }
}

void LcdDrawRoundRect(int8_t x, int8_t y, int8_t w, int8_t h, int8_t r)
{
  int8_t max_radius = ((w < h) ? w : h) / 2; // 1/2 minor axis
  if (r > max_radius)
    r = max_radius;
  // smarter version
  FastHLine(x + r, x + r + w - 2 * r, y);         // Top
  FastHLine(x + r, x + r + w - 2 * r, y + h - 1); // Bottom
  FastVLine(y + r, y + r + h - 2 * r, x);         // Left
  FastVLine(y + r, y + r + h - 2 * r, x + w - 1); // Right
  // draw four corners
  LCDDrawCircleHelper(x + r, y + r, r, 1);
  LCDDrawCircleHelper(x + w - r - 1, y + r, r, 2);
  LCDDrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4);
  LCDDrawCircleHelper(x + r, y + h - r - 1, r, 8);
}

/**************************************************************************/
/*!
   @brief    Draw a circle with filled color
    @param    x0   Center-point x coordinate
    @param    y0   Center-point y coordinate
    @param    r   Radius of circle
    @param    color 16-bit 5-6-5 Color to fill with
*/
/**************************************************************************/
void LcdFillCircle(int8_t x0, int8_t y0, int8_t r) {
  FastVLine(y0 - r, y0 - r + (2 * r + 1), x0);
  fillCircleHelper(x0, y0, r, 3, 0);
}


void LcdDrawBitmap(int16_t x, int16_t y, const uint8_t bitmap[], int16_t w, int16_t h)
{
    int16_t j, i;
    uint8_t data;
#if 0
    for (j = 0; j < h; j++) {
      for (i = 0; i < w; i++) {
          LCDWriteFB(x + i, y + j, bitmap[i + j*w]);
      }
    }
#endif

    for (j=0;j<h;j+=8)
    {
        for(i = 0; i < w; i++) {
            data = bitmap[i + w*j];
            uint16_t k;
            for(k=0;k<h;k++){
                LCDWriteFB(x+i, y+(j*8)+k, (data >> k) ^ 0x1);
            }
        }
    }
//    for (j=0;j<8;j++)
//    {
//        LCDWriteFB(x+i, y+j, 0x0);
//    }
}

/**
 * Full line draw. Non optimal version.
 * @param x0 Start X position.
 * @param y0 Start Y position.
 * @param x1 End X position.
 * @param y1 End Y position.
 */
void LcdDrawLine(int8_t x0, int8_t y0, int8_t x1, int8_t y1)
{
    float dx = x1 - x0;
    float dy = (y1 - y0);
    float step;

    if (abs(dx) >= abs(dy))
        step = abs(dx);
    else
        step = abs(dy);

    dx = dx / step;
    dy = dy / step;

    float x = x0;
    float y = y0;
    float i = 1;
    while (i <= step)
    {
      if (x>84) x -= 84;
      if (y>48) y -= 48;
      //frame_buffer[(int8_t)x][(int8_t)y] = color;
      LCDWriteFB((int8_t)x, (int8_t)y, color);
      x = x + dx;
      y = y + dy;
      i = i + 1;
    }
}

/**
 * Show character on defined position (upper left corner).
 * @param c Character to show.
 * @param x X position.
 * @param y Y position.
 */
void LcdShowChar(uint8_t c, uint8_t x, uint8_t y, uint8_t scale, uint8_t invert) {
    uint8_t i;
    uint8_t data;
    uint8_t j;
    if (y > MAX_Y - 8) y = MAX_Y - 8;
    if (scale == 1)
    {
        for(i = 0; i < 5; i++) {
            data = font_8x5[c - 0x20][i];
            for (j=0;j<8;j++)
            {
                if (invert){
                    LCDWriteFB(x+i, y+j, ((data >> j) ^ 0x1) & 0x1);
                }else{
                    LCDWriteFB(x+i, y+j, (data >> j) & 0x1);
                }
            }
        }
        for (j=0;j<8;j++)
        {
            if (invert){
                LCDWriteFB(x+i, y+j, 0x1);
            }else{
                LCDWriteFB(x+i, y+j, 0x0);
            }
        }
    }else{
        for(i = 0; i < 5; i++) {
            data = font_8x5[c - 0x20][i];
            for (j=0;j<8;j++)
            {
                if ((data >> j) & 0x1)
                {
                    LcdFillRect(x+i*scale, y+j*scale, scale, scale);
                }
                //LCDWriteFB(, (data >> y_index) & 0x1);
            }
        }
        for (j=0;j<8;j++)
        {
            uint8_t col_tmp = color;
            color = 0x0;
            LcdFillRect(x+i*scale, y+j*scale, 1, scale);
            color = col_tmp;
        }
    }
}

/**
 * Outputs string to display.
 * @param s String to show.
 * @param x X position.
 * @param y Y position.
 */
void LcdPrintf(uint8_t *s, uint8_t x, uint8_t y, uint8_t scale, uint8_t invert) {
    while(*s) {
        LcdShowChar(*s, x, y, scale, invert);
        x += 1+5*scale;
        s++;
    }
}

/**
 * Set foreground color.
 * @param c color to be used.
 */
void LcdSetFgColor(uint8_t c)
{
    color = c;
}

/**
 * Get foreground color.
 * @return Current foreground color.
 */
uint8_t LcdGetFgColor(void)
{
    return color;
}

void LcdInit(void)
{
    // SSI0 config
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0))
    {
    }
    MAP_SSIConfigSetExpClk(SSI0_BASE, MAP_SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 1000000, 8);
    MAP_SSIEnable(SSI0_BASE);

    // reset display
    SchedulerWaitMs(10);
    GPIOPinWrite(LCD_PORT, LCD_nRES, LCD_nRES); // nRESET = 1
    GPIOPinWrite(LCD_PORT, LCD_nRES, 0); // nRESET = 0
    SchedulerWaitMs(100);
    GPIOPinWrite(LCD_PORT, LCD_nRES, LCD_nRES); // nRESET = 1

    // init display
    LCDwrite(0x21, COMMAND); // power up + hor. adress + ext. mode
    LCDwrite(0x04, COMMAND);
    LCDwrite(0x14, COMMAND);
    LCDwrite(0x20, COMMAND);
    LCDwrite(DISPLAY_MODE, COMMAND);

    LcdSetContrast(LCD_CONTRAST);

    //  show stripes
    LCDClear(0x00);

    // clear buffer
    memset(frame_buffer, 0x0, sizeof(frame_buffer));
}

