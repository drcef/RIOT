#ifndef PERIPH_CONF_H_
#define PERIPH_CONF_H_

#include <stdint.h>

#include "cpu.h"
#include "periph_cpu.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief   External oscillator and clock configuration
 *
 * For selection of the used CORECLOCK, we have implemented two choices:
 *
 * - usage of the PLL fed by the internal 8MHz oscillator divided by 8
 * - usage of the internal 8MHz oscillator directly, divided by N if needed
 *
 *
 * The PLL option allows for the usage of a wider frequency range and a more
 * stable clock with less jitter. This is why we use this option as default.
 *
 * The target frequency is computed from the PLL multiplier and the PLL divisor.
 * Use the following formula to compute your values:
 *
 * CORECLOCK = ((PLL_MUL + 1) * 1MHz) / PLL_DIV
 *
 * NOTE: The PLL circuit does not run with less than 32MHz while the maximum PLL
 *       frequency is 96MHz. So PLL_MULL must be between 31 and 95!
 *
 *
 * The internal Oscillator used directly can lead to a slightly better power
 * efficiency to the cost of a less stable clock. Use this option when you know
 * what you are doing! The actual core frequency is adjusted as follows:
 *
 * CORECLOCK = 8MHz / DIV
 *
 * NOTE: A core clock frequency below 1MHz is not recommended
 *
 * @{
 */
#define CLOCK_USE_PLL       (1)

#if CLOCK_USE_PLL
/* edit these values to adjust the PLL output frequency */
#define CLOCK_PLL_MUL       (47U)               /* must be >= 31 & <= 95 */
#define CLOCK_PLL_DIV       (1U)                /* adjust to your needs */
/* generate the actual used core clock frequency */
#define CLOCK_CORECLOCK     (((CLOCK_PLL_MUL + 1) * 1000000U) / CLOCK_PLL_DIV)
#else
/* edit this value to your needs */
#define CLOCK_DIV           (1U)
/* generate the actual core clock frequency */
#define CLOCK_CORECLOCK     (8000000 / CLOCK_DIV)
#endif
/** @} */

/**
 * @name Timer peripheral configuration
 * @{
 */
#define TIMER_NUMOF         (2U)
#define TIMER_0_EN          1
#define TIMER_1_EN          1

/* Timer 0 configuration */
#define TIMER_0_DEV         TC3->COUNT16
#define TIMER_0_CHANNELS    2
#define TIMER_0_MAX_VALUE   (0xffff)
#define TIMER_0_ISR         isr_tc3

/* Timer 1 configuration */
#define TIMER_1_DEV         TC4->COUNT32
#define TIMER_1_CHANNELS    2
#define TIMER_1_MAX_VALUE   (0xffffffff)
#define TIMER_1_ISR         isr_tc4
/** @} */

/**
 * @name UART configuration
 * @{
 */
static const uart_conf_t uart_config[] = {
    {
        .dev    = &SERCOM2->USART,
        .rx_pin = GPIO_PIN(PA,15),
        .tx_pin = GPIO_PIN(PA,14),
        .mux    = GPIO_MUX_C,
        .rx_pad = UART_PAD_RX_3,
        .tx_pad = UART_PAD_TX_2
    },
    {
        .dev    = &SERCOM1->USART,
        .rx_pin = GPIO_PIN(PA,19),
        .tx_pin = GPIO_PIN(PA,18),
        .mux    = GPIO_MUX_C,
        .rx_pad = UART_PAD_RX_3,
        .tx_pad = UART_PAD_TX_2
    }
};

/* interrupt function name mapping */
#define UART_0_ISR          isr_sercom2
#define UART_1_ISR          isr_sercom1


#define UART_NUMOF          (sizeof(uart_config) / sizeof(uart_config[0]))
/** @} */

/**
 * @name PWM configuration
 * @{
 */
#define PWM_0_EN            0
#define PWM_1_EN            0
#define PWM_MAX_CHANNELS    0
//
/* number of devices that are actually defined */
#define PWM_NUMOF           (0U)

// should be [0] but gives warnings so make it an empty [1] array
static const pwm_conf_t pwm_config[1];
/** @} */

/**
 * @name SPI configuration
 * @{
 */

// SERCOM4 is the radio
// SERCOM3 is the external flash
// SERCOM1 is available on the module connector
static const spi_conf_t spi_config[] = {
	{
        .dev      = &SERCOM4->SPI,
        .miso_pin = GPIO_PIN(PC, 19),
        .mosi_pin = GPIO_PIN(PB, 30),
        .clk_pin  = GPIO_PIN(PC, 18),
        .miso_mux = GPIO_MUX_F,
        .mosi_mux = GPIO_MUX_F,
        .clk_mux  = GPIO_MUX_F,
        .miso_pad = SPI_PAD_MISO_0,
        .mosi_pad = SPI_PAD_MOSI_2_SCK_3
    },
    {
        .dev      = &SERCOM3->SPI,
        .miso_pin = GPIO_PIN(PA,24),
        .mosi_pin = GPIO_PIN(PA,25),
        .clk_pin  = GPIO_PIN(PA,28),
        .miso_mux = GPIO_MUX_C,
        .mosi_mux = GPIO_MUX_C,
        .clk_mux  = GPIO_MUX_C,
        .miso_pad = SPI_PAD_MISO_2,
        .mosi_pad = SPI_PAD_MOSI_3_SCK_1
    },
    {
        .dev      = &SERCOM1->SPI,
        .miso_pin = GPIO_PIN(PA, 18),
        .mosi_pin = GPIO_PIN(PA, 19),
        .clk_pin  = GPIO_PIN(PA, 17),
        .miso_mux = GPIO_MUX_C,
        .mosi_mux = GPIO_MUX_C,
        .clk_mux  = GPIO_MUX_C,
        .miso_pad = SPI_PAD_MISO_2,
        .mosi_pad = SPI_PAD_MOSI_3_SCK_1
    }
};

#define SPI_NUMOF           (sizeof(spi_config) / sizeof(spi_config[0]))
/** @} */

/**
 * @name I2C configuration
 * @{
 */
#define I2C_NUMOF          (1U)
#define I2C_0_EN            1
#define I2C_1_EN            0
#define I2C_2_EN            0
#define I2C_3_EN            0
#define I2C_IRQ_PRIO        1

#define I2C_0_DEV           SERCOM1->I2CM
#define I2C_0_IRQ           SERCOM1_IRQn
#define I2C_0_ISR           isr_sercom1
/* I2C 0 GCLK */
#define I2C_0_GCLK_ID       SERCOM1_GCLK_ID_CORE
#define I2C_0_GCLK_ID_SLOW  SERCOM1_GCLK_ID_SLOW
/* I2C 0 pin configuration */
#define I2C_0_SDA           GPIO_PIN(PA, 16)
#define I2C_0_SCL           GPIO_PIN(PA, 17)
#define I2C_0_MUX           GPIO_MUX_C

/**
 * @name RTC configuration
 * @{
 */
#define RTC_NUMOF           (1U)
#define RTC_DEV             RTC->MODE2
/** @} */

/**
 * @name RTT configuration
 * @{
 */
#define RTT_NUMOF           (1U)
#define RTT_DEV             RTC->MODE0
#define RTT_IRQ             RTC_IRQn
#define RTT_IRQ_PRIO        10
#define RTT_ISR             isr_rtc
#define RTT_MAX_VALUE       (0xffffffff)
#define RTT_FREQUENCY       (32768U)    /* in Hz. For changes see `rtt.c` */
#define RTT_RUNSTDBY        (1)         /* Keep RTT running in sleep states */


#define ADC_NUMOF           0

#ifdef __cplusplus
}
#endif

#endif /* PERIPH_CONF_H_ */
