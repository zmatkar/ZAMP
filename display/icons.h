/*
 * icons.h
 *
 *  Created on: 15. 3. 2021
 *      Author: zmatkar
 */

#ifndef DISPLAY_ICONS_H_
#define DISPLAY_ICONS_H_

const uint8_t phones[] = {0x9, 0x8,
0x0, 0x0, 0x1, 0x1, 0x1, 0x1, 0x1, 0x0, 0x0,
0x0, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0,
0x1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1,
0x1, 0x1, 0x1, 0x0, 0x0, 0x0, 0x1, 0x1, 0x1,
0x1, 0x1, 0x1, 0x0, 0x0, 0x0, 0x1, 0x1, 0x1,
0x1, 0x1, 0x1, 0x0, 0x0, 0x0, 0x1, 0x1, 0x1,
0x0, 0x1, 0x1, 0x0, 0x0, 0x0, 0x1, 0x1, 0x0,
0x0, 0x1, 0x1, 0x0, 0x0, 0x0, 0x1, 0x1, 0x0
};

const uint8_t repro[] = {0x8, 0x8,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x0,
0x0, 0x1, 0x1, 0x1, 0x1, 0x0, 0x1, 0x0,
0x0, 0x1, 0x0, 0x1, 0x0, 0x0, 0x1, 0x0,
0x0, 0x1, 0x0, 0x1, 0x0, 0x0, 0x1, 0x0,
0x0, 0x1, 0x1, 0x1, 0x1, 0x0, 0x1, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x1, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0,
};

const uint8_t clean[] = {0x8, 0x8,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
};


#define PROGMEM

// 'repro_8x8', 8x8px
const unsigned char icon_repro_8x8 [] PROGMEM = {
    0xc3, 0xdb, 0xdb, 0xc3, 0xbd, 0x7e, 0x00, 0xff
};
// 'cross_8x8', 8x8px
const unsigned char icon_cross_8x8 [] PROGMEM = {
    0x7e, 0xbd, 0xdb, 0xe7, 0xe7, 0xdb, 0xbd, 0x7e
};

// 'cross_12x8', 12x8px
const unsigned char icon_cross_12x8 [] PROGMEM = {
    0x7e, 0xbd, 0xbd, 0xdb, 0xdb, 0xe7, 0xe7, 0xdb, 0xdb, 0xbd, 0xbd, 0x7e
};
// 'phones_8x8', 8x8px
const unsigned char icon_phones_8x8 [] PROGMEM = {
    0xc3, 0x9d, 0x0e, 0xfe, 0xfe, 0x0e, 0x9d, 0xc3
};
// 'pointer_8x8', 8x8px
const unsigned char icon_pointer_8x8 [] PROGMEM = {
    0xfb, 0xf1, 0xe0, 0xfb, 0xfb, 0xfb, 0xff, 0xff
};
// 'pointer_up_8x8', 8x8px
const unsigned char icon_pointer_up_8x8 [] PROGMEM = {
    0xfb, 0xf9, 0xe0, 0xf9, 0xfb, 0xff, 0xff, 0xff
};
// 'pointer_right_8x8', 8x8px
const unsigned char icon_pointer_right_8x8 [] PROGMEM = {
    0xfb, 0xfb, 0xe0, 0xf1, 0xfb, 0xff, 0xff, 0xff
};
// 'pointer_right_8x8', 8x8px
const unsigned char icon_pointer_right_8x8_2 [] PROGMEM = {
    0xfb, 0xe0, 0xf1, 0xfb, 0xff, 0xff, 0xff, 0xff
};
// 'balance_8x8', 8x8px
const unsigned char icon_balance_8x8 [] PROGMEM = {
    0x08, 0x79, 0x7b, 0xff, 0xff, 0x0b, 0xa9, 0x58
};

#endif /* DATA_ICONS_H_ */