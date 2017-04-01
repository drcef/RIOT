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
 * @brief       Driver for the MCP9808 sensor
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 *
 * @}
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "periph/i2c.h"
#include "mcp9808.h"
#include "mcp9808_regs.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

#define I2C_SPEED           I2C_SPEED_FAST

#define BUS                 (dev->params.i2c)
#define ADDR                (dev->params.addr)

int mcp9808_shutdown(mcp9808_t *dev)
{
    char buf[2];
    uint16_t val;
    i2c_acquire(BUS);
    if (i2c_read_regs(BUS, ADDR, MCP9808_REG_CONFIGURATION, buf, sizeof(buf)) < 0) {
        i2c_release(BUS);
        return MCP9808_NODEV;
    }
    val = ((buf[0] << 8) | buf[1]);
    val |= MCP9808_REG_CONFIG_SHUTDOWN;
    buf[0] = val >> 8;
    buf[1] = 0xFF & val;
    if (i2c_write_regs(BUS, ADDR, MCP9808_REG_CONFIGURATION, buf, sizeof(buf)) < 0) {
        i2c_release(BUS);
        return MCP9808_NODEV;
    }

    i2c_release(BUS);
    return MCP9808_OK;
}

int mcp9808_wakeup(mcp9808_t *dev)
{
    char buf[2];
    uint16_t val;
    i2c_acquire(BUS);
    if (i2c_read_regs(BUS, ADDR, MCP9808_REG_CONFIGURATION, buf, sizeof(buf)) < 0) {
        i2c_release(BUS);
        return MCP9808_NODEV;
    }
    val = ((buf[0] << 8) | buf[1]);
    val &= ~MCP9808_REG_CONFIG_SHUTDOWN;
    buf[0] = val >> 8;
    buf[1] = 0xFF & val;
    if (i2c_write_regs(BUS, ADDR, MCP9808_REG_CONFIGURATION, buf, sizeof(buf)) < 0) {
        i2c_release(BUS);
        return MCP9808_NODEV;
    }

    i2c_release(BUS);
    return MCP9808_OK;
}

int mcp9808_verify_device(mcp9808_t *dev)
{
    char man_id[2];
    char dev_id[2];

    i2c_acquire(BUS);
    if (i2c_read_regs(BUS, ADDR, MCP9808_REG_MANUF_ID, &man_id, sizeof(man_id)) < 0) {
        i2c_release(BUS);
        return MCP9808_NODEV;
    }
    if (i2c_read_regs(BUS, ADDR, MCP9808_REG_DEVICE_ID, &dev_id, sizeof(dev_id)) < 0) {
        i2c_release(BUS);
        return MCP9808_NODEV;
    }
    i2c_release(BUS);

    if ((man_id[1] == MCP9808_MANUF_ID) && (dev_id[0] == MCP9808_DEV_ID)) {
        return MCP9808_OK;
    }
    else {
        return MCP9808_NODEV;
    }
}

int mcp9808_set_resolution(mcp9808_t *dev, uint8_t res)
{
    i2c_acquire(BUS);
    if (i2c_write_reg(BUS, ADDR, MCP9808_REG_RESOLUTION, res) < 0) {
        i2c_release(BUS);
        return MCP9808_NODEV;
    }
    i2c_release(BUS);
    return MCP9808_OK;
}

int mcp9808_init(mcp9808_t *dev, const mcp9808_params_t *params)
{
    /* write device descriptor */
    memcpy(dev, params, sizeof(mcp9808_params_t));

    i2c_acquire(BUS);
    if (i2c_init_master(BUS, I2C_SPEED) < 0) {
        i2c_release(BUS);
        DEBUG("[mcp9808] init - error: unable to initialize I2C bus\n");
        return MCP9808_NOI2C;
    }
    i2c_release(BUS);

    if(mcp9808_wakeup(dev) < 0) {
        DEBUG("[mcp9808] init - error: device wakeup failed\n");
        return MCP9808_NODEV;
    }

    if (mcp9808_verify_device(dev) < 0) {
        DEBUG("[mcp9808] init - error: device verification failed\n");
        return MCP9808_NODEV;
    }

    if (mcp9808_set_resolution(dev, params->resolution) < 0) {
        DEBUG("[mcp9808] init - error: resolution config failed\n");
        return MCP9808_NODEV;
    }

    return MCP9808_OK;
}

// Returns °C (x100)
int16_t mcp9808_read_temp(mcp9808_t *dev)
{
    char buf[2];
    int16_t raw_temp;
    float temp;

    i2c_acquire(BUS);
    if (i2c_read_regs(BUS, ADDR, MCP9808_REG_AMBIENT_TEMP, buf, sizeof(buf)) < 0) {
        i2c_release(BUS);
        return MCP9808_NODEV;
    }
    i2c_release(BUS);

    buf[0] &= 0x1F; // clear flag bits
    if (buf[0] & 0x10) buf[0] |= 0xE0; // extend sign
    raw_temp = (int16_t)((buf[0] << 8) | buf[1]); // cast to int
    temp = raw_temp / 16.0; // convert
    
    return (int16_t)(temp * 100);
}
