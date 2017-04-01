/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_mcp9808
 * @{
 *
 * @file
 * @brief       Default configuration for MCP9808 devices
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef MCP9808_PARAMS_H
#define MCP9808_PARAMS_H

#include "board.h"
#include "mcp9808.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   Set default configuration parameters for the MCP9808 driver
 * @{
 */
#ifndef MCP9808_PARAM_I2C
#define MCP9808_PARAM_I2C       	(I2C_DEV(0))
#endif
#ifndef MCP9808_PARAM_ADDR
#define MCP9808_PARAM_ADDR		(MCP9808_I2C_ADDRESS)
#endif
#ifndef MCP9808_PARAM_RESOLUTION
#define MCP9808_PARAM_RESOLUTION 	(MCP9808_RES_12BIT)
#endif

#ifndef MCP9808_PARAMS
#define MCP9808_PARAMS          { .i2c 		= MCP9808_PARAM_I2C, \
                                .addr		= MCP9808_PARAM_ADDR, \
                                .resolution = MCP9808_PARAM_RESOLUTION }
#endif
/**@}*/

/**
 * @brief   MCP9808 configuration
 */
static const mcp9808_params_t mcp9808_params[] =
{
    MCP9808_PARAMS
};


#ifdef __cplusplus
}
#endif

#endif /* MCP9808_PARAMS_H */
/** @} */
