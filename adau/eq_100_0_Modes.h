/*
 * File:           eq_100_0_Modes.h
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




ADI_REG_TYPE EQ_100_0_0[4] = {0x00, 0x80, 0x00, 0x00};
ADI_REG_TYPE EQ_100_0_1[4] = {0xFF, 0x01, 0x2F, 0x4C};
ADI_REG_TYPE EQ_100_0_2[4] = {0x00, 0x7E, 0xD2, 0x19};
ADI_REG_TYPE EQ_100_0_3[4] = {0x00, 0xFE, 0xD0, 0xB4};
ADI_REG_TYPE EQ_100_0_4[4] = {0xFF, 0x81, 0x2D, 0xE7};
ADI_REG_TYPE EQ_100_0_5[4] = {0x00, 0x80, 0x00, 0x00};
ADI_REG_TYPE EQ_100_0_6[4] = {0xFF, 0x01, 0x2F, 0x4C};
ADI_REG_TYPE EQ_100_0_7[4] = {0x00, 0x7E, 0xD2, 0x19};
ADI_REG_TYPE EQ_100_0_8[4] = {0x00, 0xFE, 0xD0, 0xB4};
ADI_REG_TYPE EQ_100_0_9[4] = {0xFF, 0x81, 0x2D, 0xE7};

void EQ_100_0_download(){
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x0019, 6, EQ_100_0_0);			/* EQ1940Dual10B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001A, 6, EQ_100_0_1);			/* EQ1940Dual11B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001B, 6, EQ_100_0_2);			/* EQ1940Dual12B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001C, 6, EQ_100_0_3);			/* EQ1940Dual11A1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001D, 6, EQ_100_0_4);			/* EQ1940Dual12A1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x0019, 6, EQ_100_0_5);			/* EQ1940Dual10B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001A, 6, EQ_100_0_6);			/* EQ1940Dual11B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001B, 6, EQ_100_0_7);			/* EQ1940Dual12B1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001C, 6, EQ_100_0_8);			/* EQ1940Dual11A1 */
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x001D, 6, EQ_100_0_9);			/* EQ1940Dual12A1 */
}
