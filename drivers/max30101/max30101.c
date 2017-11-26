/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     drivers_max30101
 * @{
 *
 * @file
 * @brief       Driver implementation for the MAX30101 Pulse Oximeter
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "periph/i2c.h"
#include "max30101.h"
#include "max30101_regs.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

#define I2C_SPEED           I2C_SPEED_FAST

#define BUS                 (dev->params.i2c)
#define ADDR                (dev->params.addr)

int max30101_init(max30101_t *dev, const max30101_params_t *params) {
	uint8_t res = 0;

	assert(dev && params);

	/* write device descriptor */
	memcpy(dev, params, sizeof(max30101_params_t));

	/* initialize the I2C bus */
    i2c_acquire(BUS);
    if (i2c_init_master(BUS, I2C_SPEED) < 0) {
        i2c_release(BUS);
        DEBUG("[max30101] init - error: unable to initialize I2C bus\n");
        return MAX30101_NOI2C;
    }

	/* check part ID */
	uint8_t part_id;
	if (i2c_read_reg(BUS, ADDR, MAX30101_REG_PARTID, &part_id) != 1) {
		i2c_release(BUS);
		DEBUG("[max30101] init - error: device unresponsive\n");
		return MAX30101_NODEV;
	}

	DEBUG("[max30101] init: part ID is %02x\n", part_id);

	/* init sequence */

	// TODO

	i2c_release(BUS);

	return MAX30101_OK;
}