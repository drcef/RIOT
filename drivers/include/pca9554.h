/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_pca9554 PCA9554 I/O Expander
 * @ingroup     **** TBD ****
 * @brief       **** TO WRITE ****
 *
 * @{
 *
 * @file
 * @brief       API for the PCA9554 sensor driver.
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef PCA9554_H
#define PCA9554_H

#include <stdint.h>
#include "periph/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PCA9554_I2C_ADDRESS
#define PCA9554_I2C_ADDRESS           0x39 /**< Default address */
#endif

/**
 * @brief   Named return values
 */
enum {
    PCA9554_OK          =  0,       /**< everything was fine */
    PCA9554_DATA_READY  =  1,       /**< new data ready to be read */
    PCA9554_NOI2C       = -1,       /**< I2C communication failed */
    PCA9554_NODEV       = -2,       /**< no PCA9554 device found on the bus */
    PCA9554_NODATA      = -3,       /**< no data available */
    PCA9554_READERROR   = -999,     /**< I2C read error */
    PCA9554_OTHERERROR  = -998      /**< fatal error */
};

enum {
    PCA9554_LOW    = 0,
    PCA9554_HIGH   = 1
};

/**
 * @brief   Configuration parameters
 */
typedef struct {
    i2c_t i2c;                  /**< I2C bus the device is connected to */
    uint8_t addr;               /**< I2C bus address of the device */
} pca9554_params_t;

/**
 * @brief   Device descriptor
 */
typedef struct {
    pca9554_params_t params;    /**< device configuration parameters */
} pca9554_t;


/**
 * @brief   Initialize the PCA9554 device.
 *
 * @param[out] dev          Device descriptor of device to initialize
 * @param[in]  params       Configuration parameters
 *
 * @return                  PCA9554_OK on success
 * @return                  PCA9554_NOI2C if initialization of I2C bus failed
 * @return                  PCA9554_NODEV if device test failed
 */
int pca9554_init(pca9554_t *dev, const pca9554_params_t *params);

/**
 * @brief   Set direction of I/O port pins
 *
 * @param[in] dev           Device descriptor
 * @param[in] val           Direction value
 *
 * @return                  PCA9554_NODEV if device communication failed
 * @return                  PCA9554_OTHERERROR if parameters are invalid
 * @return                  PCA9554_READERROR if device times out
 */
int pca9554_write_direction(pca9554_t *dev, uint8_t val);

/**
 * @brief   Read I/O port
 *
 * @param[in] dev           Device descriptor
 *
 * @return                  I/O port value
 */
uint8_t pca9554_read_port(pca9554_t *dev);

/**
 * @brief   Write I/O port
 *
 * @param[in] dev           Device descriptor
 * @param[in] val           Value to write to I/O port
 *
 * @return                  PCA9554_NODEV if device communication failed
 * @return                  PCA9554_OTHERERROR if parameters are invalid
 * @return                  PCA9554_READERROR if device times out
 */
int pca9554_write_port(pca9554_t *dev, uint8_t val);

/**
 * @brief   Read I/O pin
 *
 * @param[in] dev           Device descriptor
 * @param[in] pin           Pin number
 *
 * @return                  I/O pin value
 */
int pca9554_read_pin(pca9554_t *dev, uint8_t pin);

/**
 * @brief   Write I/O pin
 *
 * @param[in] dev           Device descriptor
 * @param[in] pin           Pin number
 * @param[in] val           Value to write to pin
 *
 * @return                  PCA9554_NODEV if device communication failed
 * @return                  PCA9554_OTHERERROR if parameters are invalid
 * @return                  PCA9554_READERROR if device times out
 */
int pca9554_write_pin(pca9554_t *dev, uint8_t pin, uint8_t val);


#ifdef __cplusplus
}
#endif

#endif /* PCA9554_H */
/** @} */
