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
/*! @file BMA400.h
 @brief Sensor driver for BME400 sensor */
/*!
 * @defgroup BMA400 BMA400 SENSOR API
 * @{*/
#ifndef BMA400_H
#define BMA400_H

#include "libmftypes.h"
#include <stdbool.h>
#include "i2c.h"
#include "libmfi2cm.h"
#include "BMA400_defs.h"


/**
* \fn bool BMA400_isReady(void).
* \brief Checking if sensor is found and ready.
* The function is reading CHIPID (0x00) register value of the sensor.
* If we have an answer, the chip is found and if it's equal to 0x90, it's ready (default reset value).
* \return 1 if ready, 0 if not.
*/
bool BMA400_isReady(void);

/**
* \fn enum powerMode BMA400_getPowerMode(void).
* \brief Read and return the powermode value written into the register of the sensor (BMA400_STATUS 0x03).
* \return enum powerMode : SLEEP, LOW POWER or NORMAL.
*/
enum powerMode BMA400_getPowerMode(void);

/**
* \fn void BMA400_init(void)
* \brief Setup of all sensor registers you need for application (power mode, range, data rate, interrupt).
* Need to be change depending your application.
*/
void BMA400_init(void);

/**
* \fn void BMA400_setPowerMode(enum powerMode mode).
* \brief Set the sensor power mode. (Sleep, Low Power or Normal).
* Low Power at least if you want data. Check the datasheet of the sensor for more information.
* \param[in] powerMode you need to setup.
*/
void BMA400_setPowerMode(enum powerMode mode);

/**
* \fn void BMA400_setRange(enum rangeAcc range)
* \brief Set the sensor data range. (2G,4G,8G,16G).
* The value need to be changed depending your application. See datasheet for more information.
* \param[in] rangeAcc you need to setup.
*/
void BMA400_setRange(enum rangeAcc range);

/**
* \fn void BMA400_setDataRate(enum dataRate odr).
* \brief Set the sensor data rate.
* Need to be changed depending your application and you MCU. See datasheet for more information.
* \param[in] dataRate you need to setup.
*/
void BMA400_setDataRate(enum dataRate odr);

/**
* \fn void BMA400_getAcc(float* x, float* y, float* z).
* \brief Read acceleration value into the sensor register.
* \param[in] *x : pointer to a float for X axis value.
* \param[in] *y : pointer to a float for Y axis value.
* \param[in] *z : pointer to a float for Z axis value.
*/
void BMA400_getAcc(float* x, float* y, float* z);

/**
* \fn uint8_t BMA400_getID(void)
* \brief Read the 0x00 (CHIPID) sensor register value.
* This function is called for the BMA400_isReady() routine.
* \return Value of the register CHIPID (0x00).
*/
uint8_t BMA400_getID(void);

/**
* \fn void BMA400_write8(uint8_t reg, uint8_t val).
* \brief Generic function using I2C MCU Driver to write into a sensor register.
* \param[in] reg : Sensor register address you want to write.
* \param[in] val : Value you want to write into reg.
*/
void BMA400_write8(uint8_t reg, uint8_t val);

/**
* \fn uint8_t BMA400_read8(uint8_t reg).
* \brief Generic function using I2C MCU Driver to read a sensor register.
* \param[in] reg : Sensor register address you want to read.
* \return Value of the register.
*/
uint8_t BMA400_read8(uint8_t reg);

/**
* \fn void BMA400_read(uint8_t reg, uint8_t *buf, uint16_t len).
* \brief Generic function using I2C MCU Driver to read multiple register value.
* The function is starting reading at reg address and increment the address to have the right number of value (len).
* This function is used in BMA_getAcc(float* x, float* y, float* z).
* \param[in] reg : Starting register address to read.
* \param[in] *buf : Pointer to an array to store the value.
* \param[in] len : number of value you want to read.
*/
void BMA400_read(uint8_t reg, uint8_t *buf, uint16_t len);

#endif // BMA400_H
/** @}*/
