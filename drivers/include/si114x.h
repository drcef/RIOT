/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_si114x SI114X Proximity & Ambient/UV Light Sensor driver
 * @ingroup     drivers_sensors
 * @brief       Driver for the digital SI114X Proximity & Ambient/UV Light
 *              sensor series. This driver can initialize the sensor in any of
 *              its available resolutions. Humidity & temperature measurements
 *              are performed in no-hold mode (see datasheet), to ensure
 *              non-blocking operations. 
 *
 * @{
 *
 * @file
 * @brief       API for the SI114X sensor driver.
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef SI114X_H
#define SI114X_H

#include <stdint.h>
#include "periph/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SI114X_I2C_ADDRESS
#define SI114X_I2C_ADDRESS      0x60 /**< Default address */
#endif

#define SI114X_HW_KEY           0x17

#define SI114X_UCOEF0_DEFAULT   0x7B
#define SI114X_UCOEF1_DEFAULT   0x6B
#define SI114X_UCOEF2_DEFAULT   0x01
#define SI114X_UCOEF3_DEFAULT   0x00

/**
 * @brief   Named return values
 */
enum {
    SI114X_OK          =  0,       /**< everything was fine */
    SI114X_DATA_READY  =  1,       /**< new data ready to be read */
    SI114X_NOI2C       = -1,       /**< I2C communication failed */
    SI114X_NODEV       = -2,       /**< no SI114X device found on the bus */
    SI114X_NODATA      = -3,       /**< no data available */
    SI114X_READERROR   = -999,     /**< I2C read error */
    SI114X_OTHERERROR  = -998      /**< fatal error */
};

/**
 * @brief   Devices supported by this driver
 */
enum {
    SI114X_TYPE_SI1141 = 0x41,
    SI114X_TYPE_SI1142 = 0x42,
    SI114X_TYPE_SI1143 = 0x43,
    SI114X_TYPE_SI1145 = 0x45,
    SI114X_TYPE_SI1146 = 0x46,
    SI114X_TYPE_SI1147 = 0x47,
};
/**
 * @brief   Configuration parameters
 */
typedef struct {
    i2c_t i2c;                  /**< I2C bus the device is connected to */
    uint8_t addr;               /**< I2C bus address of the device */
    uint8_t type;               /**< device type */
} si114x_params_t;

/**
 * @brief   Device descriptor
 */
typedef struct {
    si114x_params_t params;    /**< device configuration parameters */
} si114x_t;


int si114x_init(si114x_t *dev, const si114x_params_t *params);
uint16_t si114x_read_visible(si114x_t *dev);
uint16_t si114x_read_infrared(si114x_t *dev);
uint16_t si114x_read_uvindex(si114x_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* SI114X_H */
/** @} */
