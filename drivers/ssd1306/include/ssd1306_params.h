/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_ssd1306
 * @{
 *
 * @file
 * @brief       Default configuration for SSD1306 devices
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef SSD1306_PARAMS_H
#define SSD1306_PARAMS_H

#include "board.h"
#include "ssd1306.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @name    Set default configuration parameters for the SSD1306 driver
 * @{
 */
#ifndef SSD1306_PARAM_I2C
#define SSD1306_PARAM_I2C       (I2C_DEV(0))
#endif
#ifndef SSD1306_PARAM_ADDR
#define SSD1306_PARAM_ADDR      (SSD1306_I2C_ADDRESS)
#endif

#ifndef SSD1306_PARAMS
#define SSD1306_PARAMS          { .i2c    = SSD1306_PARAM_I2C, \
                                  .addr   = SSD1306_PARAM_ADDR }
#endif
/**@}*/

/**
 * @brief   SSD1306 configuration
 */
static const ssd1306_params_t ssd1306_params[] =
{
    SSD1306_PARAMS
};

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_PARAMS_H */
/** @} */
