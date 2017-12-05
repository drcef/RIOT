/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_max17055 MAX17055 Fuel Gauge
 * @ingroup     drivers_sensors
 * @brief       Driver for the Maxim MAX17055 Fuel Gauge IC.
 *
 * @{
 *
 * @file
 * @brief       Interface definition for the MAX17055 Fuel Gauge IC driver.
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef MAX17055_H
#define MAX17055_H

#include "periph/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAX17055_I2C_ADDRESS
#define MAX17055_I2C_ADDRESS           0x6C
#endif

/**
 * @brief   Named return values
 */
enum {
	MAX17055_OK          =  0,
	MAX17055_DATA_READY  =  1,
	MAX17055_NOI2C       = -1,
	MAX17055_NODEV       = -2,
	MAX17055_NODATA      = -3
};

/**
 * @brief   Configuration parameters
 */
typedef struct {
	i2c_t i2c;
	uint8_t addr;
	uint16_t capacity;
	uint16_t rsense;
	int16_t ichgterm;
} max17055_params_t;

/**
 * @brief   Device descriptor for MAX17055
 */
typedef struct {
	max17055_params_t params;    /**< device configuration parameters */
} max17055_t;


int max17055_init(max17055_t *dev, const max17055_params_t *params);
int max17055_repsoc(const max17055_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* MAX17055_H */
/** @} */
