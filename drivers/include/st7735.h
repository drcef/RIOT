/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    drivers_st7735 ST7735 LCD driver
 * @ingroup     drivers_actuators
 * @brief       Driver for TFT displays with the ST7735 driver IC
 *
 * @{
 *
 * @file
 * @brief       Interface definition for the ST7735 TFT driver
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 */

#ifndef ST7735_H
#define ST7735_H

#include <stdint.h>

#include "periph/gpio.h"
#include "periph/spi.h"

#ifdef __cplusplus
 extern "C" {
#endif

/**
 * @name    Definition of display dimensions
 * @{
 */
#define ST7735_RES_X                   (128U)   /**< pixels per row */
#define ST7735_RES_Y                   (128U)   /**< pixels per column */
#define ST7735_COLSTART                (2U)   /**< characters per row */
#define ST7735_ROWSTART                (3U)    /**< characters per column */
/** @} */

/**
 * @brief   ST7735 device descriptor
 */
typedef struct {
    spi_t spi;          /**< SPI bus the display is connected to */
    gpio_t cs;          /**< chip-select pin, low: active */
    gpio_t reset;       /**< reset pin, low: active */
    gpio_t mode;        /**< mode pin: low: cmd mode, high: data mode */
    uint8_t inverted;   /**< internal flag to keep track of inversion state */
    uint8_t colstart;
    uint8_t rowstart;
    uint8_t xstart;
    uint8_t ystart;
    uint8_t width;      /* These last 4 might need to be signed ints instead ... */
    uint8_t height;     /* Try it in case of strange issues */
    uint8_t cursor_x;
    uint8_t cursor_y;
    bool textwrap;
    uint16_t bg_color;
} st7735_t;

typedef struct {
    const unsigned char *index;
    const unsigned char *unicode;
    const unsigned char *data;
    unsigned char version;
    unsigned char reserved;
    unsigned char index1_first;
    unsigned char index1_last;
    unsigned char index2_first;
    unsigned char index2_last;
    unsigned char bits_index;
    unsigned char bits_width;
    unsigned char bits_height;
    unsigned char bits_xoffset;
    unsigned char bits_yoffset;
    unsigned char bits_delta;
    unsigned char line_space;
    unsigned char cap_height;
} st7735_font_t;

/**
 * @brief   Initialize the given display
 *
 * @param[in] dev           device descriptor of display to use
 * @param[in] spi           SPI bus the display is connected to
 * @param[in] cs            GPIO pin that is connected to the CS pin
 * @param[in] reset         GPIO pin that is connected to the RESET pin
 * @param[in] mode          GPIO pin that is connected to the MODE pin
 *
 * @return      0 on success
 * @return      <0 on error
 */
int st7735_init(st7735_t *dev, spi_t spi, gpio_t cs, gpio_t reset, gpio_t mode);
void st7735_set_addr_window(const st7735_t *dev, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void st7735_push_color(const st7735_t *dev, uint16_t color, uint16_t pixelcount);
void st7735_fill_screen(const st7735_t *dev, uint16_t color);
void st7735_draw_pixel(const st7735_t *dev, int16_t x, int16_t y, uint16_t color);
void st7735_draw_v_line(const st7735_t *dev, int16_t x, int16_t y, int16_t h, uint16_t color);
void st7735_draw_h_line(const st7735_t *dev, int16_t x, int16_t y, int16_t w, uint16_t color);
void st7735_fill_round_rect(const st7735_t *dev, int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, uint16_t color);
void st7735_fill_rect(const st7735_t *dev, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void st7735_set_rotation(st7735_t *dev, uint8_t r);
void st7735_invert_display(const st7735_t *dev, bool i);
uint16_t st7735_color_565(uint8_t r, uint8_t g, uint8_t b);
void st7735_draw_font_char(st7735_t *dev, const st7735_font_t *font, uint16_t color, bool transparent, unsigned int c);
void st7735_print(st7735_t *dev, const st7735_font_t *font, uint16_t color, bool transparent, const char *str);
uint8_t st7735_char_width(const st7735_font_t *font, unsigned int c, bool offset);
uint16_t st7735_str_width(const st7735_font_t *font, const char *str);

#ifdef __cplusplus
}
#endif

#endif /* ST7735_H */
/** @} */
