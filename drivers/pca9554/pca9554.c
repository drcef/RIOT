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
 * @brief       Driver for the Sensirion pca9554 sensor series
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 *
 * @}
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "assert.h"
#include "periph/i2c.h"
#include "pca9554.h"
#include "pca9554_regs.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

#define I2C_SPEED           I2C_SPEED_FAST

#define BUS                 (dev->params.i2c)
#define ADDR                (dev->params.addr)

int pca9554_init(pca9554_t *dev, const pca9554_params_t *params)
{
    /* write device descriptor */
    memcpy(dev, params, sizeof(pca9554_params_t));

    /* initialize the I2C bus */
    i2c_acquire(BUS);
    if (i2c_init_master(BUS, I2C_SPEED) < 0) {
        i2c_release(BUS);
        DEBUG("[pca9554] init - error: unable to initialize I2C bus\n");
        return PCA9554_NOI2C;
    }

    i2c_release(BUS);
    return PCA9554_OK;
}

int pca9554_write_direction(pca9554_t *dev, uint8_t val)
{
    i2c_acquire(BUS);
    i2c_write_reg(BUS, ADDR, PCA9554_CONFIG_REG, val);
    i2c_release(BUS);

    // todo check write result?

    return PCA9554_OK;
}

uint8_t pca9554_read_port(pca9554_t *dev)
{
    uint8_t val;
    
    i2c_acquire(BUS);
    i2c_read_reg(BUS, ADDR, PCA9554_INPUT_PORT_REG, &val);
    i2c_release(BUS);

    // todo check read result?

    return val;
}

int pca9554_write_port(pca9554_t *dev, uint8_t val)
{
    i2c_acquire(BUS);
    i2c_write_reg(BUS, ADDR, PCA9554_OUTPUT_PORT_REG, val);
    i2c_release(BUS);

    // todo check write result?

    return PCA9554_OK;
}

int pca9554_read_pin(pca9554_t *dev, uint8_t pin)
{
    if (pin > 7) return PCA9554_OTHERERROR;

    return (pca9554_read_port(dev) & (1 << pin)) ? 1: 0;
}

int pca9554_write_pin(pca9554_t *dev, uint8_t pin, uint8_t val)
{
    // sanity checks
    if (pin > 7) return PCA9554_OTHERERROR;
    val = val ? 1: 0;

    // read current port value
    uint8_t oldval = pca9554_read_port(dev);

    // flip pin value only if new value is different
    if (((oldval & (1 << pin)) >> pin) != val) {
        pca9554_write_port(dev, (oldval ^ (1 << pin)));
    }

    return PCA9554_OK;
}