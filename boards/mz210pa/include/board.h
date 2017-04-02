#ifndef BOARD_H_
#define BOARD_H_

#include "cpu.h"
#include "periph_conf.h"
#include "periph_cpu.h"
#include "div.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   xtimer configuration
 * @{
 */
#define XTIMER_DEV                     TIMER_RTT
#define XTIMER_CHAN                    (0)
#define XTIMER_HZ                      32768UL
#define XTIMER_USEC_TO_TICKS(value)    ( div_u32_by_15625div512(value) )
#define XTIMER_TICKS_TO_USEC(value)    ( ((uint64_t)value * 15625)>>9 )
/** @} */

/**
 * @name AT86RF233 configuration
 *
 * {spi bus, spi speed, cs pin, int pin, reset pin, sleep pin}
 */
#define AT86RF2XX_PARAMS_BOARD      {.spi = SPI_DEV(0), \
                                     .spi_clk = SPI_CLK_5MHZ, \
                                     .cs_pin = GPIO_PIN(PB, 31), \
                                     .int_pin = GPIO_PIN(PB, 0), \
                                     .sleep_pin = GPIO_PIN(PA, 20), \
                                     .reset_pin = GPIO_PIN(PB, 15)}


/**
 * @brief Initialize board specific hardware, including clock, LEDs and std-IO
 */
void board_init(void);

#ifdef __cplusplus
}
#endif

#endif /* BOARD_H_ */
