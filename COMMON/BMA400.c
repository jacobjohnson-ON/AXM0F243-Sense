/**
* Copyright 2019 Semiconductor Components Industries LLC (d/b/a “ON Semiconductor”).
* All rights reserved.  This software and/or documentation is licensed by ON Semiconductor
* under limited terms and conditions.  The terms and conditions pertaining to the software
* and/or documentation are available at http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf
* (“ON Semiconductor Standard Terms and Conditions of Sale, Section 8 Software”) and
* if applicable the software license agreement.  Do not use this software and/or
* documentation unless you have carefully read and you agree to the limited terms and
* conditions.  By using this software and/or documentation, you agree to the limited
* terms and conditions.
*
* THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
* ON SEMICONDUCTOR SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL,
* INCIDENTAL, OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
*/
/*! @file BMA400.c
 @brief Sensor driver for BME400 sensor */

#include "BMA400.h"

float BMAaccRange = 0;
void BMA400_init(void)
{
    BMA400_setPowerMode(NORMAL);
    BMA400_write8((uint8_t*)BMA400_INT_1_MAP,(uint8_t*)0x01); // INT1 as wake up interrupt
    BMA400_write8((uint8_t*)BMA400_INT_1_2_CTRL,(uint8_t*)0x02); // 0x02 INT1 active HIGH push/pull CMOS 0x06 for open drain
    BMA400_write8((uint8_t*)BMA400_AUTO_WAKE_UP_1,(uint8_t*)0x02); // wake up interrup enabled
    BMA400_write8((uint8_t*)BMA400_WAKE_UP_INT_CONFIG_0,(uint8_t*)0xF1); //enabling 3 axis interrupt / update ref every time and use 5 samples for threshold condition
    BMA400_write8((uint8_t*)BMA400_WAKE_UP_INT_CONFIG_1,(uint8_t*)0x05); //threshold for wakeup interrupt
    BMA400_setRange(RANGE_4G);
    BMA400_setDataRate(ODR_200);
}

bool BMA400_isReady(void)
{
    return (BMA400_getID() == 0x90);
}
enum powerMode BMA400_getPowerMode()
{
    uint8_t rslt;
    rslt = BMA400_read8(BMA400_STATUS);
    rslt |= 0b00000110;
    if (rslt == 0x00)
    {
        return SLEEP;
    }else if(rslt = 0x01)
    {
        return LOW_POWER;
    }else if (rslt == 0x02)
    {
        return NORMAL;
    }else
    {
        return 0x03; // ERROR
    }
}

uint8_t BMA400_getID(void)
{
    return BMA400_read8(BMA400_CHIPID);
}

void BMA400_setPowerMode(enum powerMode mode)
{
    uint8_t data = 0;
    data = BMA400_read8(BMA400_ACC_CONFIG_0);
    data = (data & 0xfc) | mode;
    BMA400_write8(BMA400_ACC_CONFIG_0, data);
}

void BMA400_setRange(enum rangeAcc range)
{
    uint8_t data = 0;
    switch(range)
    {
    case RANGE_2G:
        BMAaccRange = 2000;
        break;
    case RANGE_4G:
        BMAaccRange = 4000;
        break;
    case RANGE_8G:
        BMAaccRange = 8000;
        break;
    default :
        BMAaccRange = 16000;
        break;
    }
    data = BMA400_read8(BMA400_ACC_CONFIG_1);
    data = (data & 0x3f) | (range << 6);
    BMA400_write8(BMA400_ACC_CONFIG_1, data);
}

void BMA400_setDataRate(enum dataRate odr)
{
    uint8_t data = 0;
    data = BMA400_read8(BMA400_ACC_CONFIG_1);
    data = (data & 0xf0) | odr;
    BMA400_write8(BMA400_ACC_CONFIG_1, data);
}

void BMA400_getAcc(float* x, float* y, float* z)
{
    uint8_t buf[6] = {0};
    uint16_t ax = 0, ay=0, az=0;
    float value = 0;

    BMA400_read(BMA400_ACC_X_LSB, buf, 6);

    ax = buf[0] | (buf[1] << 8);
    ay = buf[2] | (buf[3] << 8);
    az = buf[4] | (buf[5] << 8);

    if(ax > 2047) ax = ax - 4096;
    if(ay > 2047) ay = ay - 4096;
    if(az > 2047) az = az - 4096;

    value = (int16_t)ax;
    *x = BMAaccRange * value / 2048;

    value = (int16_t)ay;
    *y = BMAaccRange * value / 2048;

    value = (int16_t)az;
    *z = BMAaccRange * value / 2048;
}

void BMA400_write8(uint8_t reg, uint8_t val)
{
    uint8_t data[2] = {reg,val};
    i2c1_write(BMA400_ADDRESS,data,2,0);
    i2c1_wait_until_done();
}

uint8_t BMA400_read8(uint8_t reg)
{
    uint8_t data = 0;
    uint8_t temp_reg = reg;
    i2c1_write(BMA400_ADDRESS,&temp_reg,1,0);
    i2c1_wait_until_done();
    i2c1_read(BMA400_ADDRESS,&data,sizeof(data),0);
    i2c1_wait_until_done();
    return data;
}

void BMA400_read(uint8_t reg, uint8_t* buf, uint16_t len)
{
    uint8_t temp_reg = reg;
    i2c1_write(BMA400_ADDRESS,&temp_reg,1,0);
    i2c1_wait_until_done();
    i2c1_read(BMA400_ADDRESS,buf,len,0);
    i2c1_wait_until_done();
}
