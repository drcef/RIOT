/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_max30101
 * @{
 *
 * @file
 * @brief       Default configuration for MAX30101 devices
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef MAX30101_PARAMS_H
#define MAX30101_PARAMS_H

#include "board.h"
#include "max30101.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters for the MAX30101 driver
 * @{
 */
#ifndef MAX30101_PARAM_I2C
#define MAX30101_PARAM_I2C       (I2C_DEV(0))
#endif
#ifndef MAX30101_PARAM_ADDR
#define MAX30101_PARAM_ADDR      (MAX30101_I2C_ADDRESS)
#endif

#ifndef MAX30101_PARAMS
#define MAX30101_PARAMS          { .i2c    = MAX30101_PARAM_I2C, \
								  .addr   = MAX30101_PARAM_ADDR }
#endif
/**@}*/

/**
 * @brief   MAX30101 configuration
 */
static const max30101_params_t max30101_params[] =
{
	MAX30101_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* MAX30101_PARAMS_H */
/** @} */
