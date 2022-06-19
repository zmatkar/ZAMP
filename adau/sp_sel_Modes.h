/*
 * File:           sp_sel_Modes.h
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




ADI_REG_TYPE SP_SEL_0[4] = {0x00, 0x00, 0x00, 0x00};

void SP_SEL_download(){
SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_, 0x0098, 6, SP_SEL_0);			/* StMuxSwSlew1coeffname */
}
