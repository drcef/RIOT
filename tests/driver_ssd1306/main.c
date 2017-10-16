/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
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
 * @brief       Test application for the SSD1306 display driver
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ssd1306.h"
#include "ssd1306_params.h"
#include "xtimer.h"
#include "shell.h"

static ssd1306_t dev;

static const shell_command_t shell_commands[] = {
    { NULL, NULL, NULL }
};

int main(void)
{
    puts("SSD1306 OLED display test application\n");
    puts("Initializing SSD1306 OLED... ");

    if (ssd1306_init(&dev, ssd1306_params) == SSD1306_OK) {
        puts("[OK]\n");
        return 1;
    }
    else {
        puts("[FAILED]\n");
    }

    uint8_t x;
    uint8_t y;

    while(1) {
        for (x=0; x<128; x++) {
            for (y=0; y<64; y++) {
                ssd1306_draw_pixel(&dev, x, y, 1);
                ssd1306_pushframe(&dev);
                xtimer_usleep(50 * 1000U);
            }
        }
    }

    return 0;
}
