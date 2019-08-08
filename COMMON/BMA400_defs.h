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
/*! @file BMA400_defs.h
 @brief Sensor driver for BME400 sensor */

/*!
 * @defgroup BMA400 BMA400 SENSOR API
 * @{*/

#ifndef BMA400_DEFS_H
#define BMA400_DEFS_H

#include "libmftypes.h"
#include <stdbool.h>
#include "i2c.h"
#include "libmfi2cm.h"


/* DATASHEET of the sensor can be found here : https://ae-bst.resource.bosch.com/media/_tech/media/datasheets/BST-BMA400-DS000.pdf */

#define BMA400_ADDRESS                  0x14 //0x14 if SDO connected to GND, 0x15 if VDD


/*** This is the address of all BMA400 registers we need to configure the sensor and read his data ***/
#define BMA400_CHIPID                   0x00
#define BMA400_ERR_REG                  0x02
#define BMA400_STATUS                   0x03

/*** This bloc is address to get sensor data ***/
#define BMA400_ACC_X_LSB                0x04
#define BMA400_ACC_X_MSB                0x05
#define BMA400_ACC_Y_LSB                0x06
#define BMA400_ACC_Y_MSB                0x07
#define BMA400_ACC_Z_LSB                0x08
#define BMA400_ACC_Z_MSB                0x09

#define BMA400_SENSOR_TIME_0            0x0A
#define BMA400_SENSOR_TIME_1            0x0B
#define BMA400_SENSOR_TIME_2            0x0C

#define BMA400_EVENT                    0x0D

#define BMA400_INT_STAT0                0x0E
#define BMA400_INT_STAT1                0x0F
#define BMA400_INT_STAT2                0x10
#define BMA400_TEMP_DATA                0x11

#define BMA400_FIFO_LENGTH_0            0x12
#define BMA400_FIFO_LENGTH_1            0x13
#define BMA400_FIFO_DATA                0x14

#define BMA400_STEP_CNT_0               0x15
#define BMA400_STEP_CNT_1               0x16
#define BMA400_STEP_CNT_2               0x17
#define BMA400_STEP_STAT                0x18

#define BMA400_ACC_CONFIG_0             0x19
#define BMA400_ACC_CONFIG_1             0x1A
#define BMA400_ACC_CONFIG_2             0x1B

#define BMA400_INT_CONFIG_0             0x1F
#define BMA400_INT_CONFIG_1             0x20
#define BMA400_INT_1_MAP                0x21
#define BMA400_INT_2_MAP                0x22
#define BMA400_INT_1_2_MAP              0x23
#define BMA400_INT_1_2_CTRL             0x24

#define BMA400_FIFO_CONFIG_0            0x26
#define BMA400_FIFO_CONFIG_1            0x27
#define BMA400_FIFO_CONFIG_2            0x28
#define BMA400_FIFO_PWR_CONFIG          0x29

#define BMA400_AUTO_LOW_POW_0           0x2A
#define BMA400_AUTO_LOW_POW_1           0x2B
#define BMA400_AUTO_WAKE_UP_0           0x2C
#define BMA400_AUTO_WAKE_UP_1           0x2D
#define BMA400_WAKE_UP_INT_CONFIG_0     0x2F
#define BMA400_WAKE_UP_INT_CONFIG_1     0x30
#define BMA400_WAKE_UP_INT_CONFIG_2     0x31
#define BMA400_WAKE_UP_INT_CONFIG_3     0x32
#define BMA400_WAKE_UP_INT_CONFIG_4     0x33

#define BMA400_ORIENTCH_CONFIG_0        0x35
#define BMA400_ORIENTCH_CONFIG_1        0x36
#define BMA400_ORIENTCH_CONFIG_2        0x37
#define BMA400_ORIENTCH_CONFIG_3        0x38
#define BMA400_ORIENTCH_CONFIG_4        0x39
#define BMA400_ORIENTCH_CONFIG_5        0x3A
#define BMA400_ORIENTCH_CONFIG_6        0x3B
#define BMA400_ORIENTCH_CONFIG_7        0x3C
#define BMA400_ORIENTCH_CONFIG_8        0x3D
#define BMA400_ORIENTCH_CONFIG_9        0x3E

#define BMA400_GEN_1_INT_CONFIG_0       0x3F
#define BMA400_GEN_1_INT_CONFIG_1       0x40
#define BMA400_GEN_1_INT_CONFIG_2       0x41
#define BMA400_GEN_1_INT_CONFIG_3       0x42
#define BMA400_GEN_1_INT_CONFIG_3_1     0x43
#define BMA400_GEN_1_INT_CONFIG_4       0x44
#define BMA400_GEN_1_INT_CONFIG_5       0x45
#define BMA400_GEN_1_INT_CONFIG_6       0x46
#define BMA400_GEN_1_INT_CONFIG_7       0x47
#define BMA400_GEN_1_INT_CONFIG_8       0x48
#define BMA400_GEN_1_INT_CONFIG_9       0x49

#define BMA400_GEN_2_INT_CONFIG_0       0x4A
#define BMA400_GEN_2_INT_CONFIG_1       0x4B
#define BMA400_GEN_2_INT_CONFIG_2       0x4C
#define BMA400_GEN_2_INT_CONFIG_3       0x4D
#define BMA400_GEN_2_INT_CONFIG_3_1     0x4E
#define BMA400_GEN_2_INT_CONFIG_4       0x4F
#define BMA400_GEN_2_INT_CONFIG_5       0x50
#define BMA400_GEN_2_INT_CONFIG_6       0x51
#define BMA400_GEN_2_INT_CONFIG_7       0x52
#define BMA400_GEN_2_INT_CONFIG_8       0x53
#define BMA400_GEN_2_INT_CONFIG_9       0x54

#define BMA400_ACT_CH_CONFIG_0          0x55
#define BMA400_ACT_CH_CONFIG_1          0x56

#define BMA400_TAP_CONFIG_0             0x57
#define BMA400_TAP_CONFIG_1             0x58

#define BMA400_STEP_COUNTER_CONFIG_0    0x59
#define BMA400_STEP_COUNTER_CONFIG_1    0x5A
#define BMA400_STEP_COUNTER_CONFIG_2    0x5B
#define BMA400_STEP_COUNTER_CONFIG_3    0x5C
#define BMA400_STEP_COUNTER_CONFIG_4    0x5D
#define BMA400_STEP_COUNTER_CONFIG_5    0x5E
#define BMA400_STEP_COUNTER_CONFIG_6    0x5F
#define BMA400_STEP_COUNTER_CONFIG_7    0x60
#define BMA400_STEP_COUNTER_CONFIG_8    0x61
#define BMA400_STEP_COUNTER_CONFIG_9    0x62
#define BMA400_STEP_COUNTER_CONFIG_10   0x63
#define BMA400_STEP_COUNTER_CONFIG_11   0x64
#define BMA400_STEP_COUNTER_CONFIG_12   0x65
#define BMA400_STEP_COUNTER_CONFIG_13   0x66
#define BMA400_STEP_COUNTER_CONFIG_14   0x67
#define BMA400_STEP_COUNTER_CONFIG_15   0x68
#define BMA400_STEP_COUNTER_CONFIG_16   0x69
#define BMA400_STEP_COUNTER_CONFIG_17   0x6A
#define BMA400_STEP_COUNTER_CONFIG_18   0x6B
#define BMA400_STEP_COUNTER_CONFIG_19   0x6C
#define BMA400_STEP_COUNTER_CONFIG_20   0x6D
#define BMA400_STEP_COUNTER_CONFIG_21   0x6E
#define BMA400_STEP_COUNTER_CONFIG_22   0x6F
#define BMA400_STEP_COUNTER_CONFIG_23   0x70
#define BMA400_STEP_COUNTER_CONFIG_24   0x71

#define BMA400_IF_CONF                  0x7C
#define BMA400_SELF_TEST                0x7D
#define BMA400_CMD                      0x7E

//type def for function pointer for read and write to attach to the struc BMA

/**
* \enum powerMode
* \brief list of different possible power mode for the sensor.
* See BMA400 datasheet for more information.
*/
enum powerMode // power mode
{
	SLEEP = 0x00, /*!< Stop data conversion, this is the lowest power mode. */
	LOW_POWER = 0x01, /*!< Lowest power mode with possible data conversion. Useful to reduce consumption between two data reading with I2C. */
	NORMAL = 0x02, /*!< Normal mode, used to write data into I2C bus. */
};

/**
* \enum rangeAcc
* \brief list of different possible data range of the sensor.
* See BMA400 datasheet for more information.
*/
enum rangeAcc // define the range of the measurment
{
	RANGE_2G = 0x00, //
	RANGE_4G = 0x01, //
	RANGE_8G = 0x02, //
    RANGE_16G = 0x03, //
};

/**
* \enum rangeAcc
* \brief list of different possible data rate of the sensor.
* See BMA400 datasheet for more information.
*/
enum dataRate // data rate
{
	ODR_12 = 0x00, //
	ODR_25 = 0x06, //
	ODR_50 = 0x07, //
    ODR_100 = 0x08, //
    ODR_200 = 0x09, //
    ODR_400 = 0x0A, //
    ODR_800 = 0x0B, //
};

#endif // BMA400_DEFS_H
/** @}*/
