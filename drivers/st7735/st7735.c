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

/* Transfering uint32_t's around is faster than uint8_t's */
/* 128x128x16bit = 32768 bytes = 8192 uint32_t's */
static uint32_t framebuffer[8192];

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
    dev->cursor_x = 0;
    dev->cursor_y = 0;
    dev->textwrap = false;
    dev->bg_color = 0x0000;

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
    xtimer_usleep(20 * 1000U);
    gpio_clear(reset);
    xtimer_usleep(20 * 1000U);
    gpio_set(reset);
    xtimer_usleep(20 * 1000U);

    /* acquire SPI */
    lock(dev);

    /* software reset */
    _writecommand(dev, ST7735_SWRESET);
    xtimer_usleep(10 * 1000U);

    /* out of sleep mode */
    _writecommand(dev, ST7735_SLPOUT);
    xtimer_usleep(10 * 1000U);

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
    xtimer_usleep(10 * 1000U);

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

void st7735_push_frame(const st7735_t *dev, uint16_t pixelcount)
{
    lock(dev);
    gpio_write(dev->mode, 1);
    spi_transfer_bytes(dev->spi, dev->cs, false, (uint8_t *)&framebuffer, NULL, pixelcount*2); // each pixel is 16 bits
    done(dev);
}

void st7735_push_color(const st7735_t *dev, uint16_t color, uint16_t pixelcount)
{
    color = (color >> 8) | (color << 8); // swap endianness

    if (pixelcount > 1) {
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
    else {
        lock(dev);
        gpio_write(dev->mode, 1);
        spi_transfer_bytes(dev->spi, dev->cs, false, (uint8_t *)&color, NULL, 2);
        done(dev);
    }
}

void st7735_draw_pixel(const st7735_t *dev, int16_t x, int16_t y, uint16_t color)
{
    if ((x < 0) || (x >= dev->width) || (y < 0) || (y >= dev->height)) return;
    st7735_set_addr_window(dev, x, y, x, y);
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

static inline uint32_t fetchbit(const uint8_t *p, uint32_t index)
{
    if (p[index >> 3] & (1 << (7 - (index & 7)))) return 1;
    return 0;
}

static uint32_t fetchbits_unsigned(const uint8_t *p, uint32_t index, uint32_t required)
{
    uint32_t val = 0;
    do {
        uint8_t b = p[index >> 3];
        uint32_t avail = 8 - (index & 7);
        if (avail <= required) {
            val <<= avail;
            val |= b & ((1 << avail) - 1);
            index += avail;
            required -= avail;
        } else {
            b >>= avail - required;
            val <<= required;
            val |= b & ((1 << required) - 1);
            break;
        }
    } while (required);
    return val;
}

static uint32_t fetchbits_signed(const uint8_t *p, uint32_t index, uint32_t required)
{
    uint32_t val = fetchbits_unsigned(p, index, required);
    if (val & (1 << (required - 1))) {
        return (int32_t)val - (1 << required);
    }
    return (int32_t)val;
}

static void draw_font_bits(const st7735_t *dev, uint32_t bits, uint32_t numbits, uint32_t x, uint32_t y, uint32_t repeat, uint16_t color, bool transparent)
{
    if (bits == 0) return;

    /* To render text transparently, we draw each pixel individually.
     * If transparency is not needed and the text can be drawn with a
     * monochrome background, we can draw the font bits more efficiently
     * using the framebuffer.
     */

    if (transparent) {
        do {
            uint32_t x1 = x;
            uint32_t n = numbits;
            do {
                n--;
                if (bits & (1 << n)) st7735_draw_pixel(dev, x1, y, color);
                x1++;
            } while (n > 0);
            y++;
            repeat--;
        } while (repeat);
    }
    else {
        uint16_t *fb = (uint16_t*)framebuffer;

        uint16_t background = dev->bg_color;
        color = (color >> 8) | (color << 8); // swap endianness
        background = (background >> 8) | (background << 8); // swap endianness

        uint32_t pixelcount = 0;
        for (uint32_t i=0; i<repeat; i++) {
            uint32_t n = numbits;
            do {
                if (bits & (1 << --n)) fb[pixelcount++] = color;
                else fb[pixelcount++] = background;
            } while (n > 0);
        }

        st7735_set_addr_window(dev, x, y, x+numbits-1, y+repeat-1);
        st7735_push_frame(dev, pixelcount);
    }
}

void st7735_draw_font_char(st7735_t *dev, const st7735_font_t *font, uint16_t color, bool transparent, unsigned int c)
{
    uint32_t bitoffset;
    const uint8_t *data;

    if (c == '\n') {
        dev->cursor_y += font->line_space;
        dev->cursor_x = 0;
        return;
    }

    if (c >= font->index1_first && c <= font->index1_last) {
        bitoffset = c - font->index1_first;
        bitoffset *= font->bits_index;
    } else if (c >= font->index2_first && c <= font->index2_last) {
        bitoffset = c - font->index2_first + font->index1_last - font->index1_first + 1;
        bitoffset *= font->bits_index;
    } else {
        return;
    }
    data = font->data + fetchbits_unsigned(font->index, bitoffset, font->bits_index);

    uint32_t encoding = fetchbits_unsigned(data, 0, 3);
    if (encoding != 0) return;
    uint32_t width = fetchbits_unsigned(data, 3, font->bits_width);
    bitoffset = font->bits_width + 3;
    uint32_t height = fetchbits_unsigned(data, bitoffset, font->bits_height);
    bitoffset += font->bits_height;

    int32_t xoffset = fetchbits_signed(data, bitoffset, font->bits_xoffset);
    bitoffset += font->bits_xoffset;
    int32_t yoffset = fetchbits_signed(data, bitoffset, font->bits_yoffset);
    bitoffset += font->bits_yoffset;

    uint32_t delta = fetchbits_unsigned(data, bitoffset, font->bits_delta);
    bitoffset += font->bits_delta;

    if (dev->cursor_x < 0) dev->cursor_x = 0;

    int32_t origin_x = dev->cursor_x + xoffset;
    if (origin_x < 0) {
        dev->cursor_x -= xoffset;
        origin_x = 0;
    }

    if (origin_x + (int)width > dev->width) {
        if (!dev->textwrap) return;
        origin_x = 0;
        if (xoffset >= 0) dev->cursor_x = 0;
        else dev->cursor_x = -xoffset;
        dev->cursor_y += font->line_space;
    }

    if (dev->cursor_y >= dev->height) return;

    dev->cursor_x += delta;

    int32_t origin_y = dev->cursor_y + font->cap_height - height - yoffset;
    int32_t linecount = height;
    uint32_t y = origin_y;
    while (linecount) {
        uint32_t b = fetchbit(data, bitoffset++);
        if (b == 0) {
            uint32_t x = 0;
            do {
                uint32_t xsize = width - x;
                if (xsize > 32) xsize = 32;
                uint32_t bits = fetchbits_unsigned(data, bitoffset, xsize);
                draw_font_bits(dev, bits, xsize, origin_x + x, y, 1, color, transparent);
                bitoffset += xsize;
                x += xsize;
            } while (x < width);
            y++;
            linecount--;
        } else {
            uint32_t n = fetchbits_unsigned(data, bitoffset, 3) + 2;
            bitoffset += 3;
            uint32_t x = 0;
            do {
                uint32_t xsize = width - x;
                if (xsize > 32) xsize = 32;
                uint32_t bits = fetchbits_unsigned(data, bitoffset, xsize);
                draw_font_bits(dev, bits, xsize, origin_x + x, y, n, color, transparent);
                bitoffset += xsize;
                x += xsize;
            } while (x < width);
            y += n;
            linecount -= n;
        }
    }
}

void st7735_print(st7735_t *dev, const st7735_font_t *font, uint16_t color, bool transparent, const char *str)
{
    unsigned char c;

    while (c = *(str++)) {
        st7735_draw_font_char(dev, font, color, transparent, c);
    }
}