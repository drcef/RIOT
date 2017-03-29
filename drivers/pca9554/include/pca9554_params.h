/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_pca9554
 * @{
 *
 * @file
 * @brief       Default configuration for PCA9554 devices
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef PCA9554_PARAMS_H
#define PCA9554_PARAMS_H

#include "board.h"
#include "pca9554.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Set default configuration parameters for the PCA9554 driver
 * @{
 */
#ifndef PCA9554_PARAM_I2C
#define PCA9554_PARAM_I2C       	(I2C_DEV(0))
#endif
#ifndef PCA9554_PARAM_ADDR
#define PCA9554_PARAM_ADDR		(PCA9554_I2C_ADDRESS)
#endif

#ifndef PCA9554_PARAMS
#define PCA9554_PARAMS          { .i2c 		= PCA9554_PARAM_I2C, \
                                .addr		= PCA9554_PARAM_ADDR }
#endif
/**@}*/

/**
 * @brief   PCA9554 configuration
 */
static const pca9554_params_t pca9554_params[] =
{
    PCA9554_PARAMS
};


#ifdef __cplusplus
}
#endif

#endif /* PCA9554_PARAMS_H */
/** @} */
