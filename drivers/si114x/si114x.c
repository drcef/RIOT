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
 * @brief       Driver for the SI114X sensor series
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 *
 * @}
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "periph/i2c.h"
#include "xtimer.h"
#include "si114x.h"
#include "si114x_regs.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

#define I2C_SPEED           I2C_SPEED_FAST

#define BUS                 (dev->params.i2c)
#define ADDR                (dev->params.addr)


static int _command(si114x_t *dev, uint8_t cmd)
{
    uint8_t res;

    // write NOP to command register to clear response register
    if (i2c_write_reg(BUS, ADDR, SI114X_REG_COMMAND, SI114X_CMD_NOP) < 0)
        return -1;

    // read response register to ensure it's clear
    i2c_read_reg(BUS, ADDR, SI114X_REG_RESPONSE, &res);
    if (res != 0x00) return -1;

    // write actual command into command register
    if (i2c_write_reg(BUS, ADDR, SI114X_REG_COMMAND, cmd) < 0)
        return -1;

    if (cmd == SI114X_CMD_RESET) return 0; // return now if command is RESET

    // wait max 25 ms for command completion (response becomes non-zero)
    for (int i=0; i<25; i++) {
        i2c_read_reg(BUS, ADDR, SI114X_REG_RESPONSE, &res);
        if (res) return 0;
        xtimer_usleep(1 * 1000U);
    }

    // command timeout
    return -1;
}

static uint8_t _read_ram(si114x_t *dev, uint8_t param)
{
    uint8_t val;

    // read desired parameter out of the RAM into PARAM_RD
    _command(dev, SI114X_CMD_PARAM_QUERY | param);

    // read value from PARAM_RD
    i2c_read_reg(BUS, ADDR, SI114X_REG_PARAM_RD, &val);

    return val;
}

static int _write_ram(si114x_t *dev, uint8_t param, uint8_t value)
{
    // write value to PARAM_WR register
    if (i2c_write_reg(BUS, ADDR, SI114X_REG_PARAM_WR, value) < 0)
        return -1;
    
    // set desired parameter to the value of PARAM_WR
    if (_command(dev, SI114X_CMD_PARAM_SET | param) < 0)
        return -1;

    return 0;
}

static int _reset(si114x_t *dev)
{
    if (_command(dev, SI114X_CMD_RESET) < 0) return -1;
    xtimer_usleep(30 * 1000U); // wait 30 ms for device startup

    // write HW_KEY for device to operate properly
    if (i2c_write_reg(BUS, ADDR, SI114X_REG_HW_KEY, SI114X_HW_KEY) < 0)
        return -1;

    return 0;
}

static int _verify(si114x_t *dev)
{
    uint8_t part_id;
    if (i2c_read_reg(BUS, ADDR, SI114X_REG_RESPONSE, &part_id) < 0)
        return -1;

    if (part_id == SI114X_TYPE_SI1141 ||
        part_id == SI114X_TYPE_SI1142 ||
        part_id == SI114X_TYPE_SI1143 ||
        part_id == SI114X_TYPE_SI1145 ||
        part_id == SI114X_TYPE_SI1146 ||
        part_id == SI114X_TYPE_SI1147)
        return 0;

    return -1;
}

static int _default_config(si114x_t *dev)
{
    // enable VIS, IR and UV reading in SI1145/6/7 devices
    // or VIS and IR only in SI1141/2/3 devices
    if (dev->params.type == SI114X_TYPE_SI1145 ||
        dev->params.type == SI114X_TYPE_SI1146 ||
        dev->params.type == SI114X_TYPE_SI1147)
    {
        // write default UV calibration coefficients
        if (i2c_write_reg(BUS, ADDR, SI114X_REG_UCOEF0, SI114X_UCOEF0_DEFAULT) < 0)
            return -1;
        if (i2c_write_reg(BUS, ADDR, SI114X_REG_UCOEF1, SI114X_UCOEF1_DEFAULT) < 0)
            return -1;
        if (i2c_write_reg(BUS, ADDR, SI114X_REG_UCOEF2, SI114X_UCOEF2_DEFAULT) < 0)
            return -1;
        if (i2c_write_reg(BUS, ADDR, SI114X_REG_UCOEF3, SI114X_UCOEF3_DEFAULT) < 0)
            return -1;

        // write to CHLIST to enable visible, infrared and UV reading
        if (_write_ram(dev, SI114X_RAM_CHLIST,
            SI114X_CHLIST_EN_ALS_VIS | SI114X_CHLIST_EN_ALS_IR | SI114X_CHLIST_EN_UV) < 0)
            return -1;
    }
    else {
        // write to CHLIST to enable only visible and infrared reading
        if (_write_ram(dev, SI114X_RAM_CHLIST,
            SI114X_CHLIST_EN_ALS_VIS | SI114X_CHLIST_EN_ALS_IR) < 0)
            return -1;
    }

    // All other configuration parameters remain in their default value after reset

    return 0;
}

int si114x_init(si114x_t *dev, const si114x_params_t *params)
{
    /* write device descriptor */
    memcpy(dev, params, sizeof(si114x_params_t));

    i2c_acquire(BUS);
    if (i2c_init_master(BUS, I2C_SPEED) < 0) {
        i2c_release(BUS);
        DEBUG("[si114x] init - error: unable to initialize I2C bus\n");
        return SI114X_NOI2C;
    }

    if (_verify(dev) < 0) {
        i2c_release(BUS);
        DEBUG("[si114x] init - error: device verification failed\n");
        return SI114X_NODEV;
    }

    if (_reset(dev) < 0) {
        i2c_release(BUS);
        DEBUG("[si114x] init - error: device reset failed\n");
        return SI114X_NODEV;
    }

    if (_default_config(dev) < 0) {
        i2c_release(BUS);
        DEBUG("[si114x] init - error: device config failed\n");
        return SI114X_NODEV;
    }

    i2c_release(BUS);
    return SI114X_OK;
}

uint16_t si114x_read_visible(si114x_t *dev)
{
    uint8_t buf[2];

    i2c_acquire(BUS);
    _command(dev, SI114X_CMD_ALS_FORCE);
    xtimer_usleep(350);
    i2c_read_regs(BUS, ADDR, SI114X_REG_ALS_VIS_DATA0, buf, sizeof(buf));
    i2c_release(BUS);

    return (uint16_t)((buf[1] << 8) | buf[0]);
}

uint16_t si114x_read_infrared(si114x_t *dev)
{
    uint8_t buf[2];

    i2c_acquire(BUS);
    _command(dev, SI114X_CMD_ALS_FORCE);
    xtimer_usleep(350);
    i2c_read_regs(BUS, ADDR, SI114X_REG_ALS_IR_DATA0, buf, sizeof(buf));
    i2c_release(BUS);
    
    return (uint16_t)((buf[1] << 8) | buf[0]);
}

uint16_t si114x_read_uvindex(si114x_t *dev)
{
    uint8_t buf[2];

    i2c_acquire(BUS);
    _command(dev, SI114X_CMD_ALS_FORCE);
    xtimer_usleep(350);
    i2c_read_regs(BUS, ADDR, SI114X_REG_UVINDEX0, buf, sizeof(buf));
    i2c_release(BUS);
    
    return (uint16_t)((buf[1] << 8) | buf[0]);
}