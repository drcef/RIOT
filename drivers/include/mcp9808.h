/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_mcp9808 MCP9808 Digital Temperature Sensor driver
 * @ingroup     drivers_sensors
 * @brief       To be written.
 *
 * @{
 *
 * @file
 * @brief       API for the MCP9808 sensor driver.
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef MCP9808_H
#define MCP9808_H

#include <stdint.h>
#include "periph/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MCP9808_I2C_ADDRESS
#define MCP9808_I2C_ADDRESS     0x18
#endif

#define MCP9808_MANUF_ID        0x0054
#define MCP9808_DEV_ID          0x0400

enum {
    MCP9808_OK          =  0,       /**< everything was fine */
    MCP9808_NOI2C       = -1,       /**< I2C communication failed */
    MCP9808_NODEV       = -2,       /**< no MCP9808 device found on the bus */
    MCP9808_NODATA      = -3,       /**< no data available */
    MCP9808_READERROR   = -999,     /**< I2C read error */
    MCP9808_OTHERERROR  = -998      /**< fatal error */
};

enum {
    MCP9808_RES_9BIT  = 0x00,
    MCP9808_RES_10BIT = 0x01,
    MCP9808_RES_11BIT = 0x02,
    MCP9808_RES_12BIT = 0x03
};

typedef struct {
    i2c_t i2c;
    uint8_t addr;
    uint8_t resolution;
} mcp9808_params_t;


typedef struct {
    mcp9808_params_t params;
} mcp9808_t;


int mcp9808_shutdown(mcp9808_t *dev);
int mcp9808_wakeup(mcp9808_t *dev);
int mcp9808_verify_device(mcp9808_t *dev);
int mcp9808_set_resolution(mcp9808_t *dev, uint8_t res);
int mcp9808_init(mcp9808_t *dev, const mcp9808_params_t *params);
int16_t mcp9808_read_temp(mcp9808_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* MCP9808_H */
/** @} */
