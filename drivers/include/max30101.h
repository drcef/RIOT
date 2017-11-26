/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_max30101 MAX30101 Pulse Oximeter
 * @ingroup     drivers_sensors
 * @brief       Driver for the Maxim MAX30101 Pulse Oximeter
 *
 * @{
 *
 * @file
 * @brief       Interface definition for the MAX30101 Pulse Oximeter driver
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef MAX30101_H
#define MAX30101_H

#include "periph/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX30101_I2C_ADDRESS
#define MAX30101_I2C_ADDRESS           0xAE
#endif

/**
 * @brief   Named return values
 */
enum {
	MAX30101_OK          =  0,
	MAX30101_DATA_READY  =  1,
	MAX30101_NOI2C       = -1,
	MAX30101_NODEV       = -2,
	MAX30101_NODATA      = -3
};

/**
 * @brief   Configuration parameters
 */
typedef struct {
	i2c_t i2c;
	uint8_t addr;
} max30101_params_t;

/**
 * @brief   Device descriptor for MAX30101
 */
typedef struct {
	max30101_params_t params;    /**< device configuration parameters */
} max30101_t;


int max30101_init(max30101_t *dev, const max30101_params_t *params);

#ifdef __cplusplus
}
#endif

#endif /* MAX30101_H */
/** @} */
