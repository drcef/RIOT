/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_ssd1306 SSD1306 Display Controller
 * @ingroup     drivers_actuators
 * @brief       Driver for OLED displays using the SSD1306 controller IC.
 *
 * @{
 *
 * @file
 * @brief       Interface definition for the SSD1306 Display Controller driver.
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef SSD1306_H
#define SSD1306_H

#include "periph/i2c.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SSD1306_I2C_ADDRESS
#define SSD1306_I2C_ADDRESS           0x78
#endif

/**
 * @brief   Named return values
 */
enum {
    SSD1306_OK          =  0,
    SSD1306_DATA_READY  =  1,
    SSD1306_NOI2C       = -1,
    SSD1306_NODEV       = -2,
    SSD1306_NODATA      = -3
};

/**
 * @brief   Configuration parameters
 */
typedef struct {
    i2c_t i2c;
    uint8_t addr;
} ssd1306_params_t;

/**
 * @brief   Device descriptor for SSD1306
 */
typedef struct {
    ssd1306_params_t params;    /**< device configuration parameters */
    uint8_t framebuffer[1024]; /* 128 x 64 x 1bit */ 
} ssd1306_t;


int ssd1306_init(ssd1306_t *dev, const ssd1306_params_t *params);
void ssd1306_pushframe(const ssd1306_t *dev);
void ssd1306_draw_pixel(ssd1306_t *dev, uint8_t x, uint8_t y, uint8_t val);

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_H */
/** @} */
