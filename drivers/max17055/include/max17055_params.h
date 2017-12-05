/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_max17055
 * @{
 *
 * @file
 * @brief       Default configuration for MAX17055 devices
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef MAX17055_PARAMS_H
#define MAX17055_PARAMS_H

#include "board.h"
#include "max17055.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters for the MAX17055 driver
 * @{
 */
#ifndef MAX17055_PARAM_I2C
#define MAX17055_PARAM_I2C		(I2C_DEV(0))
#endif
#ifndef MAX17055_PARAM_ADDR
#define MAX17055_PARAM_ADDR		(MAX17055_I2C_ADDRESS)
#endif
#ifndef MAX17055_PARAM_CAPACITY
#define MAX17055_PARAM_CAPACITY	(800U)
#endif
#ifndef MAX17055_PARAM_RSENSE
#define MAX17055_PARAM_RSENSE	(10U)
#endif
#ifndef MAX17055_PARAM_ICHGTERM
#define MAX17055_PARAM_ICHGTERM	(50U)
#endif

#ifndef MAX17055_PARAMS
#define MAX17055_PARAMS          { .i2c     = MAX17055_PARAM_I2C, \
								  .addr     = MAX17055_PARAM_ADDR, \
								  .capacity = MAX17055_PARAM_CAPACITY, \
								  .rsense	= MAX17055_PARAM_RSENSE, \
								  .ichgterm = MAX17055_PARAM_ICHGTERM }
#endif
/**@}*/

/**
 * @brief   MAX17055 configuration
 */
static const max17055_params_t max17055_params[] =
{
	MAX17055_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* MAX17055_PARAMS_H */
/** @} */
