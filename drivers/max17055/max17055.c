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
#include "xtimer.h"
#include "max17055.h"
#include "max17055_regs.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

#define I2C_SPEED           I2C_SPEED_FAST

#define BUS                 (dev->params.i2c)
#define ADDR                (dev->params.addr)

int max17055_init(max17055_t *dev, const max17055_params_t *params) {
	uint16_t res;
	uint16_t uval;
	int16_t sval;

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
	if (i2c_read_regs(BUS, ADDR, MAX17055_REG_STATUS, &res, sizeof(res)) != 2) {
		i2c_release(BUS);
		DEBUG("[max17055] init - error: device unresponsive\n");
		return MAX17055_NODEV;
	}

	if (res) { // initialize device
		do {
			i2c_read_regs(BUS, ADDR, MAX17055_REG_FSTAT, &res, sizeof(res));
			xtimer_usleep(10 * 1000U);
		} while (res & MAX17055_FSTAT_DNR); // wait until DNR (Data Not Ready) is cleared

		 // write DesignCap value calculated from nominal capacity and Rsense value (refer to datasheet)
		uint16_t designcap = (dev->params.capacity * dev->params.rsense) / 5;
		i2c_write_regs(BUS, ADDR, MAX17055_REG_DESIGNCAP, &designcap, sizeof(designcap));

		// write dQAcc (not sure if necessary?)
		uval = designcap / 32;
		i2c_write_regs(BUS, ADDR, MAX17055_REG_DQACC, &uval, sizeof(uval));

		// write IChgTerm value calculated from charge termination current and Rsense value (refer to datasheet)
		sval = ((dev->params.ichgterm * dev->params.rsense) * 64) / 100;
		i2c_write_regs(BUS, ADDR, MAX17055_REG_ICHGTERM, &sval, sizeof(sval));

		// temporarily disable hibernation and wake up the device to apply configuration faster
		uval = MAX17055_SOFT_WAKEUP_SET;
		i2c_write_regs(BUS, ADDR, MAX17055_REG_SOFT_WAKEUP, &uval, sizeof(uval));
		uval = MAX17055_HIBCFG_VAL & ~(MAX17055_HIBCFG_ENHIB);
		i2c_write_regs(BUS, ADDR, MAX17055_REG_HIBCFG, &uval, sizeof(uval));
		uval = MAX17055_SOFT_WAKEUP_CLEAR;
		i2c_write_regs(BUS, ADDR, MAX17055_REG_SOFT_WAKEUP, &uval, sizeof(uval));

		// write dPAcc (not sure if necessary?)
		uval = (designcap / 32) * 44138 / designcap; // refer to datasheet
		i2c_write_regs(BUS, ADDR, MAX17055_REG_DPACC, &uval, sizeof(uval));

		// select EZ Mode 0 with charge voltage < 4.275V
		uval = MAX17055_MODELCFG_REFRESH;
		i2c_write_regs(BUS, ADDR, MAX17055_REG_MODELCFG, &uval, sizeof(uval));
		
		do {
			i2c_read_regs(BUS, ADDR, MAX17055_REG_MODELCFG, &res, sizeof(res));
			xtimer_usleep(10 * 1000U);
		} while (res & MAX17055_MODELCFG_REFRESH); // wait until model is loaded (refresh bit cleared)

		// re-enable hibernation
		uval = MAX17055_HIBCFG_VAL | MAX17055_HIBCFG_ENHIB;
		i2c_write_regs(BUS, ADDR, MAX17055_REG_HIBCFG, &uval, sizeof(uval));

		// write Status with POR (Power-on Reset) bit cleared to indicate successful init
		i2c_read_regs(BUS, ADDR, MAX17055_REG_STATUS, &res, sizeof(res));
		uval = res & ~(MAX17055_STATUS_POR);
		i2c_write_regs(BUS, ADDR, MAX17055_REG_STATUS, &uval, sizeof(uval));

	}
	else {
		// device is already initialized
	}

	i2c_release(BUS);

	return MAX17055_OK;
}

int max17055_repsoc(const max17055_t *dev)
{
	uint16_t res;
	if (i2c_read_regs(BUS, ADDR, MAX17055_REG_REPSOC, &res, sizeof(res)) < 2) {
		return -1;
	}

	return (int) (res >> 8);
}