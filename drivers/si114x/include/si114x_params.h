/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_si114x
 * @{
 *
 * @file
 * @brief       Default configuration for SI114X devices
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef SI114X_PARAMS_H
#define SI114X_PARAMS_H

#include "board.h"
#include "si114x.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Set default configuration parameters for the SI114X driver
 * @{
 */
#ifndef SI114X_PARAM_I2C
#define SI114X_PARAM_I2C       	(I2C_DEV(0))
#endif
#ifndef SI114X_PARAM_ADDR
#define SI114X_PARAM_ADDR		(SI114X_I2C_ADDRESS)
#endif
#ifndef SI114X_PARAM_TYPE
#define SI114X_PARAM_TYPE		(SI114X_TYPE_SI1145)
#endif

#ifndef SI114X_PARAMS
#define SI114X_PARAMS          { .i2c 		= SI114X_PARAM_I2C, \
                                .addr		= SI114X_PARAM_ADDR, \
                                .type   	= SI114X_PARAM_TYPE }
#endif
/**@}*/

/**
 * @brief   SI114X configuration
 */
static const si114x_params_t si114x_params[] =
{
    SI114X_PARAMS
};


#ifdef __cplusplus
}
#endif

#endif /* SI114X_PARAMS_H */
/** @} */
