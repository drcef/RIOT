/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_st7735
 * @{
 * @file
 * @brief       Implementation of the SPI driver for ST7735 TFT displays
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 * @}
 */

#include <stdint.h>
#include <stdio.h>

#include "xtimer.h"
#include "periph/spi.h"
#include "periph/gpio.h"
#include "st7735.h"
#include "st7735_internal.h"

#define ENABLE_DEBUG        (1)
#include "debug.h"

#define ASCII_MIN           0x20    /**< start of ASCII table */
#define ASCII_MAX           0x7e    /**< end of ASCII table */
#define CHAR_WIDTH          (6U)    /**< pixel width of a single character */

#define SPI_CLK             (SPI_CLK_10MHZ)
#define SPI_MODE            (SPI_MODE_0)

static uint32_t framebuffer[8192];

static const uint8_t _ascii[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00},/* 20 SPACE*/
    {0x00, 0x00, 0x5f, 0x00, 0x00},/* 21 ! */
    {0x00, 0x07, 0x00, 0x07, 0x00},/* 22 " */
    {0x14, 0x7f, 0x14, 0x7f, 0x14},/* 23 # */
    {0x24, 0x2a, 0x7f, 0x2a, 0x12},/* 24 $ */
    {0x23, 0x13, 0x08, 0x64, 0x62},/* 25 % */
    {0x36, 0x49, 0x55, 0x22, 0x50},/* 26 & */
    {0x00, 0x05, 0x03, 0x00, 0x00},/* 27 ' */
    {0x00, 0x1c, 0x22, 0x41, 0x00},/* 28 ( */
    {0x00, 0x41, 0x22, 0x1c, 0x00},/* 29 ) */
    {0x14, 0x08, 0x3e, 0x08, 0x14},/* 2a * */
    {0x08, 0x08, 0x3e, 0x08, 0x08},/* 2b + */
    {0x00, 0x50, 0x30, 0x00, 0x00},/* 2c , */
    {0x08, 0x08, 0x08, 0x08, 0x08},/* 2d - */
    {0x00, 0x60, 0x60, 0x00, 0x00},/* 2e . */
    {0x20, 0x10, 0x08, 0x04, 0x02},/* 2f / */
    {0x3e, 0x51, 0x49, 0x45, 0x3e},/* 30 0 */
    {0x00, 0x42, 0x7f, 0x40, 0x00},/* 31 1 */
    {0x42, 0x61, 0x51, 0x49, 0x46},/* 32 2 */
    {0x21, 0x41, 0x45, 0x4b, 0x31},/* 33 3 */
    {0x18, 0x14, 0x12, 0x7f, 0x10},/* 34 4 */
    {0x27, 0x45, 0x45, 0x45, 0x39},/* 35 5 */
    {0x3c, 0x4a, 0x49, 0x49, 0x30},/* 36 6 */
    {0x01, 0x71, 0x09, 0x05, 0x03},/* 37 7 */
    {0x36, 0x49, 0x49, 0x49, 0x36},/* 38 8 */
    {0x06, 0x49, 0x49, 0x29, 0x1e},/* 39 9 */
    {0x00, 0x36, 0x36, 0x00, 0x00},/* 3a : */
    {0x00, 0x56, 0x36, 0x00, 0x00},/* 3b ; */
    {0x08, 0x14, 0x22, 0x41, 0x00},/* 3c < */
    {0x14, 0x14, 0x14, 0x14, 0x14},/* 3d = */
    {0x00, 0x41, 0x22, 0x14, 0x08},/* 3e > */
    {0x02, 0x01, 0x51, 0x09, 0x06},/* 3f ? */
    {0x32, 0x49, 0x79, 0x41, 0x3e},/* 40 @ */
    {0x7e, 0x11, 0x11, 0x11, 0x7e},/* 41 A */
    {0x7f, 0x49, 0x49, 0x49, 0x36},/* 42 B */
    {0x3e, 0x41, 0x41, 0x41, 0x22},/* 43 C */
    {0x7f, 0x41, 0x41, 0x22, 0x1c},/* 44 D */
    {0x7f, 0x49, 0x49, 0x49, 0x41},/* 45 E */
    {0x7f, 0x09, 0x09, 0x09, 0x01},/* 46 F */
    {0x3e, 0x41, 0x49, 0x49, 0x7a},/* 47 G */
    {0x7f, 0x08, 0x08, 0x08, 0x7f},/* 48 H */
    {0x00, 0x41, 0x7f, 0x41, 0x00},/* 49 I */
    {0x20, 0x40, 0x41, 0x3f, 0x01},/* 4a J */
    {0x7f, 0x08, 0x14, 0x22, 0x41},/* 4b K */
    {0x7f, 0x40, 0x40, 0x40, 0x40},/* 4c L */
    {0x7f, 0x02, 0x0c, 0x02, 0x7f},/* 4d M */
    {0x7f, 0x04, 0x08, 0x10, 0x7f},/* 4e N */
    {0x3e, 0x41, 0x41, 0x41, 0x3e},/* 4f O */
    {0x7f, 0x09, 0x09, 0x09, 0x06},/* 50 P */
    {0x3e, 0x41, 0x51, 0x21, 0x5e},/* 51 Q */
    {0x7f, 0x09, 0x19, 0x29, 0x46},/* 52 R */
    {0x46, 0x49, 0x49, 0x49, 0x31},/* 53 S */
    {0x01, 0x01, 0x7f, 0x01, 0x01},/* 54 T */
    {0x3f, 0x40, 0x40, 0x40, 0x3f},/* 55 U */
    {0x1f, 0x20, 0x40, 0x20, 0x1f},/* 56 V */
    {0x3f, 0x40, 0x38, 0x40, 0x3f},/* 57 W */
    {0x63, 0x14, 0x08, 0x14, 0x63},/* 58 X */
    {0x07, 0x08, 0x70, 0x08, 0x07},/* 59 Y */
    {0x61, 0x51, 0x49, 0x45, 0x43},/* 5a Z */
    {0x00, 0x7f, 0x41, 0x41, 0x00},/* 5b [ */
    {0x02, 0x04, 0x08, 0x10, 0x20},/* 5c \ */
    {0x00, 0x41, 0x41, 0x7f, 0x00},/* 5d ] */
    {0x04, 0x02, 0x01, 0x02, 0x04},/* 5e ^ */
    {0x40, 0x40, 0x40, 0x40, 0x40},/* 5f _ */
    {0x00, 0x01, 0x02, 0x04, 0x00},/* 60 ` */
    {0x20, 0x54, 0x54, 0x54, 0x78},/* 61 a */
    {0x7f, 0x48, 0x44, 0x44, 0x38},/* 62 b */
    {0x38, 0x44, 0x44, 0x44, 0x20},/* 63 c */
    {0x38, 0x44, 0x44, 0x48, 0x7f},/* 64 d */
    {0x38, 0x54, 0x54, 0x54, 0x18},/* 65 e */
    {0x08, 0x7e, 0x09, 0x01, 0x02},/* 66 f */
    {0x0c, 0x52, 0x52, 0x52, 0x3e},/* 67 g */
    {0x7f, 0x08, 0x04, 0x04, 0x78},/* 68 h */
    {0x00, 0x44, 0x7d, 0x40, 0x00},/* 69 i */
    {0x20, 0x40, 0x44, 0x3d, 0x00},/* 6a j */
    {0x7f, 0x10, 0x28, 0x44, 0x00},/* 6b k */
    {0x00, 0x41, 0x7f, 0x40, 0x00},/* 6c l */
    {0x7c, 0x04, 0x18, 0x04, 0x78},/* 6d m */
    {0x7c, 0x08, 0x04, 0x04, 0x78},/* 6e n */
    {0x38, 0x44, 0x44, 0x44, 0x38},/* 6f o */
    {0x7c, 0x14, 0x14, 0x14, 0x08},/* 70 p */
    {0x08, 0x14, 0x14, 0x18, 0x7c},/* 71 q */
    {0x7c, 0x08, 0x04, 0x04, 0x08},/* 72 r */
    {0x48, 0x54, 0x54, 0x54, 0x20},/* 73 s */
    {0x04, 0x3f, 0x44, 0x40, 0x20},/* 74 t */
    {0x3c, 0x40, 0x40, 0x20, 0x7c},/* 75 u */
    {0x1c, 0x20, 0x40, 0x20, 0x1c},/* 76 v */
    {0x3c, 0x40, 0x30, 0x40, 0x3c},/* 77 w */
    {0x44, 0x28, 0x10, 0x28, 0x44},/* 78 x */
    {0x0c, 0x50, 0x50, 0x50, 0x3c},/* 79 y */
    {0x44, 0x64, 0x54, 0x4c, 0x44},/* 7a z */
    {0x00, 0x08, 0x36, 0x41, 0x00},/* 7b { */
    {0x00, 0x00, 0x7f, 0x00, 0x00},/* 7c | */
    {0x00, 0x41, 0x36, 0x08, 0x00},/* 7d } */
    {0x10, 0x08, 0x08, 0x10, 0x08},/* 7e ~ */
};

static inline void lock(const st7735_t *dev)
{
    spi_acquire(dev->spi, dev->cs, SPI_MODE, SPI_CLK);
}

static inline void done(const st7735_t *dev)
{
    spi_release(dev->spi);
}

static inline void _spiwrite(const st7735_t *dev, char data, bool cont)
{
    spi_transfer_bytes(dev->spi, dev->cs, cont, (uint8_t *)&data, NULL, 1);
}

static void _writecommand(const st7735_t *dev, char data)
{
    /* set command mode */
    gpio_write(dev->mode, 0);
    /* write byte to LCD */
    _spiwrite(dev, data, false);
}

static void _writedata(const st7735_t *dev, char data)
{
    /* set data mode */
    gpio_write(dev->mode, 1);
    /* write byte to LCD */
    _spiwrite(dev, data, false);
}

void st7735_set_rotation(st7735_t *dev, uint8_t m)
{
    /* Can't be higher than 3 */
    uint8_t rotation = m % 4;
    
    lock(dev);

    _writecommand(dev, ST7735_MADCTL);
    switch (rotation) {
        case 0:
            _writedata(dev, (MADCTL_MX | MADCTL_MY | MADCTL_BGR));
            dev->xstart = dev->colstart;
            dev->ystart = dev->rowstart;
            dev->height = ST7735_RES_X;
            dev->width = ST7735_RES_Y;
            break;
        case 1:
            _writedata(dev, (MADCTL_MY | MADCTL_MV | MADCTL_BGR));
            dev->ystart = dev->colstart;
            dev->xstart = dev->rowstart;
            dev->height = ST7735_RES_Y;
            dev->width = ST7735_RES_X;
            break;
        case 2:
            _writedata(dev, MADCTL_BGR);
            dev->xstart = dev->colstart;
            dev->ystart = dev->rowstart;
            dev->height = ST7735_RES_X;
            dev->width = ST7735_RES_Y;
            break;
        case 3:
            _writedata(dev, MADCTL_MX | MADCTL_MV | MADCTL_BGR);
            dev->ystart = dev->colstart;
            dev->xstart = dev->rowstart;
            dev->height = ST7735_RES_Y;
            dev->width = ST7735_RES_X;
            break;
    }

    done(dev);
}

uint16_t st7735_color_565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

int st7735_init(st7735_t *dev, spi_t spi, gpio_t cs, gpio_t reset, gpio_t mode)
{
    /* save pin mapping */
    dev->spi = spi;
    dev->cs = cs;
    dev->reset = reset;
    dev->mode = mode;
    dev->inverted = 0;
    dev->colstart = ST7735_COLSTART;
    dev->rowstart = ST7735_ROWSTART;

    DEBUG("done setting dev members\n");

    /* initialze pins */
    gpio_init(reset, GPIO_OUT);
    gpio_init(mode, GPIO_OUT);
    DEBUG("done with gpios\n");
    
    /* initialize SPI */
    spi_init_cs(spi, (spi_cs_t)cs);
    DEBUG("done initializing SPI master\n");
    
    /* reset display */
    gpio_set(reset);
    xtimer_usleep(500 * 1000U);
    gpio_clear(reset);
    xtimer_usleep(500 * 1000U);
    gpio_set(reset);
    xtimer_usleep(500 * 1000U);

    /* acquire SPI */
    lock(dev);

    /* software reset */
    _writecommand(dev, ST7735_SWRESET);
    xtimer_usleep(150 * 1000U);

    /* out of sleep mode */
    _writecommand(dev, ST7735_SLPOUT);
    xtimer_usleep(500 * 1000U);

    /* frame rate control - normal mode*/
    _writecommand(dev, ST7735_FRMCTR1);
    _writedata(dev, 0x01);
    _writedata(dev, 0x2C);
    _writedata(dev, 0x2D);

    /* frame rate control - idle mode*/
    _writecommand(dev, ST7735_FRMCTR2);
    _writedata(dev, 0x01);
    _writedata(dev, 0x2C);
    _writedata(dev, 0x2D);

    /* frame rate control - partial mode*/
    _writecommand(dev, ST7735_FRMCTR3);
    _writedata(dev, 0x01);
    _writedata(dev, 0x2C);
    _writedata(dev, 0x2D);
    _writedata(dev, 0x01);
    _writedata(dev, 0x2C);
    _writedata(dev, 0x2D);

    /* Display inversion control */
    _writecommand(dev, ST7735_INVCTR);
    _writedata(dev, 0x07);

    /* Power control */
    _writecommand(dev, ST7735_PWCTR1);
    _writedata(dev, 0xA2);
    _writedata(dev, 0x02);
    _writedata(dev, 0x84);
    _writecommand(dev, ST7735_PWCTR2);
    _writedata(dev, 0xC5);
    _writecommand(dev, ST7735_PWCTR3);
    _writedata(dev, 0x0A);
    _writedata(dev, 0x00);
    _writecommand(dev, ST7735_PWCTR4);
    _writedata(dev, 0x8A);
    _writedata(dev, 0x2A);
    _writecommand(dev, ST7735_PWCTR5);
    _writedata(dev, 0x8A);
    _writedata(dev, 0xEE);
    _writecommand(dev, ST7735_VMCTR1);
    _writedata(dev, 0x0E);

    /* Don't invert display */
    _writecommand(dev, ST7735_INVOFF);

    /* Memory access control */
    _writecommand(dev, ST7735_MADCTL);
    _writedata(dev, 0xC8);

    /* Set color mode */
    _writecommand(dev, ST7735_COLMOD);
    _writedata(dev, 0x05);

    /* Column address set (128*128) */
    _writecommand(dev, ST7735_CASET);
    _writedata(dev, 0x00);
    _writedata(dev, 0x00);
    _writedata(dev, 0x00);
    _writedata(dev, 0x7F);

    _writecommand(dev, ST7735_RASET);
    _writedata(dev, 0x00);
    _writedata(dev, 0x00);
    _writedata(dev, 0x00);
    _writedata(dev, 0x7F);

    /* Magic numbers */
    _writecommand(dev, ST7735_GMCTRP1);
    _writedata(dev, 0x02);
    _writedata(dev, 0x1C);
    _writedata(dev, 0x07);
    _writedata(dev, 0x12);
    _writedata(dev, 0x37);
    _writedata(dev, 0x32);
    _writedata(dev, 0x29);
    _writedata(dev, 0x2D);
    _writedata(dev, 0x29);
    _writedata(dev, 0x25);
    _writedata(dev, 0x2B);
    _writedata(dev, 0x39);
    _writedata(dev, 0x00);
    _writedata(dev, 0x01);
    _writedata(dev, 0x03);
    _writedata(dev, 0x10);

    /* More magic numbers */
    _writecommand(dev, ST7735_GMCTRN1);
    _writedata(dev, 0x03);
    _writedata(dev, 0x1D);
    _writedata(dev, 0x07);
    _writedata(dev, 0x06);
    _writedata(dev, 0x2E);
    _writedata(dev, 0x2C);
    _writedata(dev, 0x29);
    _writedata(dev, 0x2D);
    _writedata(dev, 0x2E);
    _writedata(dev, 0x2E);
    _writedata(dev, 0x37);
    _writedata(dev, 0x3F);
    _writedata(dev, 0x00);
    _writedata(dev, 0x00);
    _writedata(dev, 0x02);
    _writedata(dev, 0x10);

    /* Normal display on */
    _writecommand(dev, ST7735_NORON);
    xtimer_usleep(10 * 1000U);

    /* Main screen turn on */
    _writecommand(dev, ST7735_DISPON);
    xtimer_usleep(100 * 1000U);

    /* release SPI */
    done(dev);

    st7735_set_rotation(dev, 0);
    
    return 0;
}

void st7735_set_addr_window(const st7735_t *dev, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    uint8_t cmd;
    uint8_t data[4] = { 0 };

    /* == OPTIMIZATION ==
     * This function is called at the beginning of every draw operation, so
     * it is worth optimizing. The following code is 70% faster than using
     * a series of _writecommand() and _writedata().
     */

    /* Endianness mindfuck ahead. Proceed with caution. */

    lock(dev);

    cmd = ST7735_CASET; /* set column address */
    data[1] = x0+(dev->xstart); data[3] = x1+(dev->xstart);
    gpio_clear(dev->mode); /* command mode */
    spi_transfer_bytes(dev->spi, dev->cs, true, &cmd, NULL, 1);
    gpio_set(dev->mode); /* data mode */
    spi_transfer_bytes(dev->spi, dev->cs, true, (uint8_t*)&data, NULL, 4);

    
    cmd = ST7735_RASET; /* set row address */
    data[1] = y0+(dev->ystart); data[3] = y1+(dev->ystart);
    gpio_clear(dev->mode);
    spi_transfer_bytes(dev->spi, dev->cs, true, &cmd, NULL, 1);
    gpio_set(dev->mode);
    spi_transfer_bytes(dev->spi, dev->cs, true, (uint8_t*)&data, NULL, 4);

    cmd = ST7735_RAMWR;
    gpio_clear(dev->mode);
    spi_transfer_bytes(dev->spi, dev->cs, false, &cmd, NULL, 1);

    done(dev);
}

void st7735_push_color(const st7735_t *dev, uint16_t color, uint16_t pixelcount)
{
    color = (color >> 8) | (color << 8); // swap endianness
    const uint32_t val = (color << 16) | color;
    uint16_t bytecount = pixelcount * 2;
    uint16_t opcount = pixelcount / 2;

    /* == OPTIMIZATION ==
     * Bulk transferring all pixels in one spi_transfer_bytes call eliminates
     * a lot of overhead compared to transferring each pixel seperately. At 10 MHz,
     * this more than triples the transmission rate of frames. The whole frame needs
     * to be rendered in a framebuffer first though, which requires sufficient SRAM.
     *
     * The copy operation to fill the framebuffer is at least twice as fast when
     * using uint32_t types compared to uint8_t because a 32-bit CPU does both in
     * in one operation but copies 4 bytes at once with uint32_t.
     */
    for (uint16_t i = 0; i < opcount; i++) {
        framebuffer[i] = val;
    }

    lock(dev);
    gpio_write(dev->mode, 1);
    spi_transfer_bytes(dev->spi, dev->cs, false, (uint8_t *)&framebuffer, NULL, bytecount);
    done(dev);
}

void st7735_draw_pixel(const st7735_t *dev, int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= dev->width) || (y < 0) || (y >= dev->height)) return;
    st7735_set_addr_window(dev, x, y, x+1, y+1);
    st7735_push_color(dev, color, 1);
}

void st7735_draw_v_line(const st7735_t *dev, int16_t x, int16_t y, int16_t h, uint16_t color)
{
    if ((x >= dev->width) || (y >= dev->height)) return;
    if ((y+h-1) >= dev->height) h = dev->height-y;
    st7735_set_addr_window(dev, x, y, x, y+h-1);
    st7735_push_color(dev, color, h);
}

void st7735_draw_h_line(const st7735_t *dev, int16_t x, int16_t y, int16_t w, uint16_t color)
{
    if ((x >= dev->width) || (y >= dev->height)) return;
    if ((x+w-1) >= dev->width)  w = dev->width-x;
    st7735_set_addr_window(dev, x, y, x+w-1, y);
    st7735_push_color(dev, color, w);
}

void st7735_fill_rect(const st7735_t *dev, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    if ((x >= dev->width) || (y >= dev->height)) return;
    if ((x + w - 1) >= dev->width)  w = dev->width  - x;
    if ((y + h - 1) >= dev->height) h = dev->height - y;
    st7735_set_addr_window(dev, x, y, x+w-1, y+h-1);
    st7735_push_color(dev, color, w*h);
}

void st7735_fill_screen(const st7735_t *dev, uint16_t color)
{
    st7735_fill_rect(dev, 0, 0, dev->width, dev->height, color);
}

void st7735_invert_display(const st7735_t *dev, bool i)
{
    lock(dev);
    _writecommand(dev, i ? ST7735_INVON : ST7735_INVOFF);
    done(dev);
}