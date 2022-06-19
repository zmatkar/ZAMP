/*
 * SigmaStudioFW.c
 *
 *  Created on: 21. 3. 2021
 *      Author: zmatkar
 */



/*
 * SigmaStudioFW.c
 *
 *  Created on: 2017/06/01
 *      Author: shun
 */

#include "SigmaStudioFW.h"
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "driverlib/rom_map.h"
#include "src/scheduler.h"
#include "Amp_v3_IC_1.h"


// wait for I2C not busy and then check for error.
#define WAIT_IDLE_RECV()     while(I2CMasterBusy(I2C0_BASE)){};CheckErrorReceive();
#define WAIT_IDLE_SEND()     while(I2CMasterBusy(I2C0_BASE)){};CheckErrorSend();

uint32_t i2c_err = 0;
uint16_t i2c_error = 0;

static void CheckErrorSend(void)
{
    i2c_err = I2CMasterErr(SIGMA_I2C);
    if (i2c_err)
    {
        I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);
        //while(i);
        //for(;;);
        i2c_error = 10;
    }
}

static void CheckErrorReceive(void)
{
    i2c_err = I2CMasterErr(SIGMA_I2C);
    if (i2c_err)
    {
        I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_RECEIVE_ERROR_STOP);
        //while(i);
        //for(;;);
        i2c_error = 10;
    }
}


int32_t SIGMA_READ_REGISTER_BLOCK(uint8_t devAddress, uint16_t address, uint16_t length, ADI_REG_U8* pData)
{
    devAddress = devAddress >> 1;

    I2CMasterSlaveAddrSet(SIGMA_I2C, devAddress, false);
    // send address first
    I2CMasterDataPut(SIGMA_I2C, address >> 8);
    I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_SEND_START);
    WAIT_IDLE_SEND();


    I2CMasterDataPut(SIGMA_I2C, address  & 0xFF);
    I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_SEND_CONT);
    WAIT_IDLE_SEND();

    I2CMasterSlaveAddrSet(SIGMA_I2C, devAddress, true);
    I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_RECEIVE_START);
    WAIT_IDLE_RECV();
    pData[0] = I2CMasterDataGet(SIGMA_I2C);

    // now data
    uint16_t i;
    for (i=1;i<(length-1);i++)
    {
        I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
        WAIT_IDLE_RECV();
        pData[i] = I2CMasterDataGet(SIGMA_I2C);
    }

    I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
    WAIT_IDLE_RECV();
    pData[i] = I2CMasterDataGet(SIGMA_I2C);

    return 0;

}


int32_t SIGMA_WRITE_REGISTER(uint8_t devAddress, uint16_t address, uint16_t dataLength, int32_t data )
{
    // not implemented
    return -1;
}

int32_t SIGMA_WRITE_REGISTER_BLOCK(uint8_t devAddress, uint16_t address, uint16_t length, ADI_REG_TYPE *pData)
{
    //I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_SEND_ERROR_STOP);

    devAddress = devAddress >> 1;

    I2CMasterSlaveAddrSet(SIGMA_I2C, devAddress, false);
    // send address first
    I2CMasterDataPut(SIGMA_I2C, address >> 8);
    I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_SEND_START);
    WAIT_IDLE_SEND();


    I2CMasterDataPut(SIGMA_I2C, address  & 0xFF);
    I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_SEND_CONT);
    WAIT_IDLE_SEND();

    // now data
    uint16_t i;
    for (i=0;i<(length - 1);i++)
    {
        I2CMasterDataPut(SIGMA_I2C, pData[i]);
        I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_SEND_CONT);
        WAIT_IDLE_SEND();
    }

    I2CMasterDataPut(SIGMA_I2C, pData[i]);
    I2CMasterControl(SIGMA_I2C, I2C_MASTER_CMD_BURST_SEND_FINISH);
    WAIT_IDLE_SEND();

    //SchedulerWaitMs(1);

    return 0;
}

int32_t SIGMA_WRITE_REGISTER_CONTROL(uint8_t devAddress, uint16_t address, uint16_t length, ADI_REG_U8 *pData)
{
    return SIGMA_WRITE_REGISTER_BLOCK(devAddress, address, length, pData);
}

int32_t SIGMA_SAFELOAD_WRITE_ADDR(uint8_t devAddress, uint16_t addrAddress, uint16_t address)
{
    uint8_t buffer[2];
    buffer[0] = (address >> 8) & 0xFF;
    buffer[1] = (address >> 0) & 0xFF;
    return SIGMA_WRITE_REGISTER_BLOCK(devAddress, addrAddress, 2, buffer);
}

int32_t SIGMA_SAFELOAD_WRITE_DATA(uint8_t devAddress, uint16_t dataAddress, uint32_t data)
{
    uint8_t buffer[5];
    int16_t i;

    //buffer[0]=0x00;

    for(i=0;i<5;i++)
    {
        buffer[i] = (data >> (8*(4-i))) & 0xFF;
    }
    return SIGMA_WRITE_REGISTER_BLOCK(devAddress, dataAddress, 5, buffer);
}

int32_t SIGMA_SAFELOAD_WRITE_TRANSFER_BIT(uint8_t devAddress)
{
    uint8_t buffer[REG_COREREGISTER_IC_1_BYTE];
    buffer[0] = R4_COREREGISTER_IC_1_Default[0];
    buffer[1] = R4_COREREGISTER_IC_1_Default[1] | R13_SAFELOAD_IC_1_MASK; //set IST bit
    return SIGMA_WRITE_REGISTER_BLOCK( DEVICE_ADDR_IC_1, REG_COREREGISTER_IC_1_ADDR, REG_COREREGISTER_IC_1_BYTE, buffer);
}

void SIGMA_LOAD_IC1(void)
{
    default_download_IC_1();
}

