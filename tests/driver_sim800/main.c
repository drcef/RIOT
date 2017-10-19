#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sim800.h"
#include "xtimer.h"

static sim800_t dev;

int main(void)
{
    puts("SIM800 GPRS test application\n");
    char http_buffer[256];

    if (sim800_init(&dev, 1) != SIM800_READY) {
        puts("sim800: init failure\n");
        return 0;
    }

    puts("sim800: init successful\n");
    if (sim800_gprs_connect(&dev) != SIM800_GPRS_READY) {
        sim800_gprs_disconnect(&dev);
        puts("sim800: GPRS error\n");
        return 0;
    }
    puts("sim800: GPRS connection established\n");
    if (sim800_http(&dev, "138.68.132.195", 3000, "GET / HTTP/1.1\r\nConnection: close\r\n\r\n", http_buffer, sizeof(http_buffer)) != SIM800_OK) {
        puts("sim800: HTTP error\n");
        sim800_gprs_disconnect(&dev);
        return 0;
    }

    printf("%s\n", http_buffer);

    sim800_gprs_disconnect(&dev);

    while(1) {
        
        xtimer_usleep(500 * 1000U);
    }

    return 0;
}
