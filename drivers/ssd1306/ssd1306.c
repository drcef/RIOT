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
 * @brief       Driver implementation for OLED displays with the SSD1306 IC
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>

#include "periph/i2c.h"
#include "ssd1306.h"
#include "ssd1306_regs.h"

#define ENABLE_DEBUG        (0)
#include "debug.h"

#define I2C_SPEED           I2C_SPEED_FAST

#define BUS                 (dev->params.i2c)
#define ADDR                (dev->params.addr)

int ssd1306_init(ssd1306_t *dev, const ssd1306_params_t *params)
{
    uint8_t res = 0;

    assert(dev && params);

    /* write device descriptor */
    memcpy(dev, params, sizeof(ssd1306_params_t));

    //memset(dev->framebuffer, 0xFF, 1024);

    /* initialize the I2C bus */
    i2c_acquire(BUS);
    if (i2c_init_master(BUS, I2C_SPEED) < 0) {
        i2c_release(BUS);
        DEBUG("[ssd1306] init - error: unable to initialize I2C bus\n");
        return SSD1306_NOI2C;
    }

    /* test if the target device responds */
    if (i2c_write_byte(BUS, ADDR, SSD1306_CONTROL) != 1) {
        i2c_release(BUS);
        DEBUG("[ssd1306] init - error: device unresponsive");
        return SSD1306_NODEV;
    }

    /* init sequence */
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_DISPLAYOFF);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_SETDISPLAYCLOCKDIV);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 0x80); /* datasheet suggested ratio */

    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_SETMULTIPLEX);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 63); /* height - 1 */

    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_SETDISPLAYOFFSET);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 0x00);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_SETSTARTLINE | 0x00);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_CHARGEPUMP);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 0x14); /* no external vcc */

    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_MEMORYMODE);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 0x00);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_SEGREMAP | 0x1);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_COMSCANDEC);

    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_SETCOMPINS);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 0x12);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_SETCONTRAST);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 0xCF); /* no external vcc */

    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_SETPRECHARGE);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 0xF1); /* no external vcc */
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_SETVCOMDETECT);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 0x40);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_DISPLAYALLON_RESUME);
    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_NORMALDISPLAY);

    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_DEACTIVATE_SCROLL);

    res += i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_DISPLAYON);

    i2c_release(BUS);

    if (res < 26) {
        DEBUG("[ssd1306] init - error: some init commands failed");
        return SSD1306_NODEV;
    }

    return SSD1306_OK;
}

void ssd1306_pushframe(const ssd1306_t *dev)
{
    i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_COLUMNADDR);
    i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 0); /* col start address */
    i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 127); /* col end address */

    i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, SSD1306_PAGEADDR);
    i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 0); /* page start address */
    i2c_write_reg(BUS, ADDR, SSD1306_CONTROL, 7); /* page end address */

    i2c_write_regs(BUS, ADDR, SSD1306_DATA, dev->framebuffer, 1024);
}

void ssd1306_draw_pixel(ssd1306_t *dev, uint8_t x, uint8_t y, uint8_t val)
{
    if (x > 127 || y > 63) return;

    if (val) dev->framebuffer[x+128*(y/8)] |= (1<<(y%8));
    else dev->framebuffer[x+128*(y/8)] &= ~(1<<(y%8));
}