/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     drivers_ssd1306
 * @{
 *
 * @file
 * @brief       Driver implementation for the MAX17055 fuel gauge IC
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "periph/i2c.h"
#include "max17055.h"
#include "max17055_regs.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

#define I2C_SPEED           I2C_SPEED_FAST

#define BUS                 (dev->params.i2c)
#define ADDR                (dev->params.addr)

int max17055_init(max17055_t *dev, const max17055_params_t *params) {
	uint8_t res = 0;

	assert(dev && params);

	/* write device descriptor */
	memcpy(dev, params, sizeof(max17055_params_t));

	/* initialize the I2C bus */
    i2c_acquire(BUS);
    if (i2c_init_master(BUS, I2C_SPEED) < 0) {
        i2c_release(BUS);
        DEBUG("[max17055] init - error: unable to initialize I2C bus\n");
        return MAX17055_NOI2C;
    }

	/* check POR (power on reset) */
	uint8_t por;
	if (i2c_read_reg(BUS, ADDR, MAX17055_REG_STATUS, &por) != 1) {
		i2c_release(BUS);
		DEBUG("[max17055] init - error: device unresponsive\n");
		return MAX17055_NODEV;
	}

	if (por) {
		/* initialize device */
		DEBUG("[max17055] init: POR is 1\n");
	}
	else {
		/* device is already initialized */
		DEBUG("[max17055] init: POR is 0\n");
	}

	/* init sequence */

	// TODO

	i2c_release(BUS);

	return MAX17055_OK;
}