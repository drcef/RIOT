/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     tests
 * @{
 *
 * @file
 * @brief       Test application for the ST7735 display driver
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#ifndef TEST_ST7735_SPI
#error "TEST_ST7735_SPI not defined"
#endif
#ifndef TEST_ST7735_CS
#error "TEST_ST7735_CS not defined"
#endif
#ifndef TEST_ST7735_RESET
#error "TEST_ST7735_RESET not defined"
#endif
#ifndef TEST_ST7735_MODE
#error "TEST_ST7735_MODE not defined"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "st7735.h"
#include "xtimer.h"
#include "shell.h"

static st7735_t dev;

static const shell_command_t shell_commands[] = {
    { NULL, NULL, NULL }
};

static const char rainbow[] = {
    0XFF, 0X00, 0X00,
    0XFF, 0X00, 0X31,
    0XFF, 0X00, 0X62,
    0XFF, 0X00, 0X93,
    0XFF, 0X00, 0XC5,
    0XFF, 0X00, 0XF6,
    0XD6, 0X00, 0XFF,
    0XA4, 0X00, 0XFF,
    0X73, 0X00, 0XFF,
    0X42, 0X00, 0XFF,
    0X00, 0X20, 0XFF,
    0X00, 0X51, 0XFF,
    0X00, 0X83, 0XFF,
    0X00, 0XB4, 0XFF,
    0X00, 0XE5, 0XFF,
    0X00, 0XFF, 0XE6,
    0X00, 0XFF, 0XB5,
    0X00, 0XFF, 0X84,
    0X00, 0XFF, 0X52,
    0X00, 0XFF, 0X21,
    0X0F, 0XFF, 0X00,
    0X41, 0XFF, 0X00,
    0X72, 0XFF, 0X00,
    0XA3, 0XFF, 0X00,
    0XD5, 0XFF, 0X00,
    0XFF, 0XF7, 0X00,
    0XFF, 0XC6, 0X00,
    0XFF, 0X94, 0X00,
    0XFF, 0X63, 0X00,
    0XFF, 0X32, 0X00
};

int main(void)
{
    puts("ST7735 TFT display test application\n");
    printf("Initializing ST7735 TFT at SPI_%i... ", TEST_ST7735_SPI);
    if (st7735_init(&dev, TEST_ST7735_SPI, TEST_ST7735_CS, TEST_ST7735_RESET, TEST_ST7735_MODE) != 0) {
        puts("Failed to initialize ST7735 display\n");
        return 1;
    }
    puts("Successful!\n");

    uint16_t green = st7735_color_565(0x00, 0xFF, 0x00);
    uint16_t black = st7735_color_565(0x00, 0x00, 0x00);
    uint16_t white = st7735_color_565(0xFF, 0xFF, 0xFF);

    uint8_t i, x, y, w, h;
    uint16_t color;

    while(1) {
        st7735_fill_screen(&dev, black);

        for (x=0; x<128; x+=6) {
            st7735_draw_v_line(&dev, x, 0, dev.height, white);
            xtimer_usleep(100 * 1000U);
        }

        for (y=0; y<128; y+=6) {
            st7735_draw_h_line(&dev, 0, y, dev.width, white);
            xtimer_usleep(100 * 1000U);
        }

        xtimer_usleep(500 * 1000U);
        st7735_fill_screen(&dev, black);

        i = 0;
        x = 0;
        y = 0;
        while ((x<64) && (y<64)) {
            w = 128 - x*2;
            h = 128 - y*2;

            color = st7735_color_565(rainbow[i], rainbow[i+1], rainbow[i+2]);
            st7735_fill_rect(&dev, x, y, w, h, color);
            
            x += 4;
            y += 4;
            i += 1;

            xtimer_usleep(100 * 1000U);
        }

        xtimer_usleep(500 * 1000U);

        for (i=0; i<5; i++) {
            st7735_invert_display(&dev, true);
            xtimer_usleep(100 * 1000U);
            st7735_invert_display(&dev, false);
            xtimer_usleep(100 * 1000U);
        }
        
        xtimer_usleep(500 * 1000U);
    }

    return 0;
}
