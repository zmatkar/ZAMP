/*
 * File:           eq_100_max_Modes.h
 *
 * Created:        Wednesday, March 24, 2021 9:58:48 PM
 * Description:    Amp_v3 Sequence data definitions.
 *
 * This software is distributed in the hope that it will be useful,
 * but is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * This software may only be used to program products purchased from
 * Analog Devices for incorporation by you into audio products that
 * are intended for resale to audio product end users. This software
 * may not be distributed whole or in any part to third parties.
 *
 * Copyright (c) 2021 Analog Devices, Inc. All rights reserved.
 */




ADI_REG_TYPE EQ_100_MAX_0[4] = {0x00, 0x80, 0x58, 0xA0};
ADI_REG_TYPE EQ_100_MAX_1[4] = {0xFF, 0x00, 0xE4, 0x4B};
ADI_REG_TYPE EQ_100_MAX_2[4] = {0x00, 0x7E, 0xC5, 0x91};
ADI_REG_TYPE EQ_100_MAX_3[4] = {0x00, 0xFF, 0x1C, 0x8F};
ADI_REG_TYPE EQ_100_MAX_4[4] = {0xFF, 0x80, 0xE2, 0xA8};
ADI_REG_TYPE EQ_100_MAX_5[4] = {0x00, 0x80, 0x58, 0xA0};
ADI_REG_TYPE EQ_100_MAX_6[4] = {0xFF, 0x00, 0xE4, 0x4B};
ADI_REG_TYPE EQ_100_MAX_7[4] = {0x00, 0x7E, 0xC5, 0x91};
ADI_REG_TYPE EQ_100_MAX_8[4] = {0x00, 0xFF, 0x1C, 0x8F};
ADI_REG_TYPE EQ_100_MAX_9[4] = {0xFF, 0x80, 0xE2, 0xA8};

void EQ_100_MAX_download(){
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x0019, 6, EQ_100_MAX_0);			/* EQ1940Dual10B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001A, 6, EQ_100_MAX_1);			/* EQ1940Dual11B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001B, 6, EQ_100_MAX_2);			/* EQ1940Dual12B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001C, 6, EQ_100_MAX_3);			/* EQ1940Dual11A1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001D, 6, EQ_100_MAX_4);			/* EQ1940Dual12A1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x0019, 6, EQ_100_MAX_5);			/* EQ1940Dual10B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001A, 6, EQ_100_MAX_6);			/* EQ1940Dual11B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001B, 6, EQ_100_MAX_7);			/* EQ1940Dual12B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001C, 6, EQ_100_MAX_8);			/* EQ1940Dual11A1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001D, 6, EQ_100_MAX_9);			/* EQ1940Dual12A1 */
}
