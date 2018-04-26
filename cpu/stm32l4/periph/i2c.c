/*
 * Copyright (C) 2017 Georgios Psimenos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdint.h>


#include "cpu.h"
#include "mutex.h"
#include "periph/i2c.h"
#include "periph/gpio.h"
#include "periph_conf.h"


#define ENABLE_DEBUG    (0)
#include "debug.h"


/* guard file in case no I2C device is defined */
#if I2C_NUMOF

#define MAX_NBYTE_SIZE      255U

#define I2C_RELOAD_MODE                I2C_CR2_RELOAD
#define I2C_AUTOEND_MODE               I2C_CR2_AUTOEND
#define I2C_SOFTEND_MODE               (0x00000000U)

#define I2C_NO_STARTSTOP               (0x00000000U)
#define I2C_GENERATE_STOP              I2C_CR2_STOP
#define I2C_GENERATE_START_READ        (uint32_t)(I2C_CR2_START | I2C_CR2_RD_WRN)
#define I2C_GENERATE_START_WRITE       I2C_CR2_START

#define I2C_STATUS_OK       0
#define I2C_STATUS_ERROR    1

/* static function definitions */
static void _i2c_init(I2C_TypeDef *i2c, uint32_t timingr);
static void _i2c_transfer_config(I2C_TypeDef *i2c, uint8_t address, uint8_t length, uint32_t mode, uint32_t request);
static uint8_t _wait_for_stop(I2C_TypeDef *i2c);

/**
 * @brief Array holding one pre-initialized mutex for each I2C device
 */
static mutex_t locks[] =  {
#if I2C_0_EN
    [I2C_0] = MUTEX_INIT,
#endif
#if I2C_1_EN
    [I2C_1] = MUTEX_INIT,
#endif
#if I2C_2_EN
    [I2C_2] = MUTEX_INIT,
#endif
#if I2C_3_EN
    [I2C_3] = MUTEX_INIT
#endif
};

int i2c_init_master(i2c_t dev, i2c_speed_t speed)
{
    uint32_t timingr;

    if ((unsigned int)dev >= I2C_NUMOF) {
        return -1;
    }

    /* read speed configuration */
    switch (speed) {
        case I2C_SPEED_NORMAL:
            timingr = 0x00404C74; /**** ONLY VALID FOR 80MHz CORECLOCK ****/
            break;

        case I2C_SPEED_FAST:
            timingr = 0x00100822; /**** ONLY VALID FOR 80MHz CORECLOCK ****/
            break;

        default:
            DEBUG("unsup speed %u!\n", speed);
            return -2;
    }
    I2C_TypeDef *i2c = i2c_config[dev].dev;

    /* enable I2C clock */
    i2c_poweron(dev);

    /* set IRQn priority */
    NVIC_SetPriority(i2c_config[dev].er_irqn, I2C_IRQ_PRIO);

    /* enable IRQn */
    NVIC_EnableIRQ(i2c_config[dev].er_irqn);

    /* configure pins */
    gpio_init(i2c_config[dev].scl, i2c_config[dev].pin_mode);
    gpio_init_af(i2c_config[dev].scl, i2c_config[dev].af);
    gpio_init(i2c_config[dev].sda, i2c_config[dev].pin_mode);
    gpio_init_af(i2c_config[dev].sda, i2c_config[dev].af);

    /* configure device */
    _i2c_init(i2c, timingr);

    return 0;
}

static void _i2c_init(I2C_TypeDef *i2c, uint32_t timingr)
{
    /* enable error interrupt */
    i2c->CR1 |= I2C_CR1_ERRIE;
    /* configure I2C clock */
    i2c->TIMINGR = timingr;
    /* configure device */
    i2c->OAR1 = I2C_OAR1_OA1EN; /* makes sure we are in 7-bit address mode */
    /* enable device */
    i2c->CR1 |= I2C_CR1_PE;
}

static void _i2c_transfer_config(I2C_TypeDef *i2c, uint8_t address, uint8_t length, uint32_t mode, uint32_t request)
{
    uint32_t tmpreg = 0U;
    /* get current control register 2 contents */
    tmpreg = i2c->CR2;
    /* clear bits we care about */
    tmpreg &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP));
    /* update tmpreg */
    tmpreg |= (uint32_t)(((uint32_t)address & I2C_CR2_SADD) | (((uint32_t)length << 16) & I2C_CR2_NBYTES) | (uint32_t)mode | (uint32_t)request);
    /* update control register */
    i2c->CR2 = tmpreg;
}

static inline void _clear_cr2(I2C_TypeDef *i2c)
{
    i2c->CR2 &= (uint32_t)~((uint32_t)(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN));
}

static uint8_t _check_ack_status(I2C_TypeDef *i2c)
{
    if (i2c->ISR & I2C_ISR_NACKF) {
        /* wait until STOP flag is raised */
        /* autoend should be initiate after ack fail */
        while (!(i2c->ISR & I2C_ISR_STOPF)) {}
        /* clear NACK flag */
        i2c->ICR = I2C_ICR_NACKCF;
        /* clear STOPF flag */
        i2c->ICR = I2C_ICR_STOPCF;
        /* flush TXDR */
        if (i2c->ISR & I2C_ISR_TXIS) i2c->TXDR = 0x00; /* write dummy byte if transmit pending */
        if (!(i2c->ISR & I2C_ISR_TXE)) i2c->ISR |= I2C_ISR_TXE; /* flush pending byte if TXDR not empty */
        /* clear configuration register 2 */
        _clear_cr2(i2c);
        return I2C_STATUS_ERROR;
    }
    return I2C_STATUS_OK;
}

static inline uint8_t _wait_for_stop(I2C_TypeDef *i2c)
{
    while (!(i2c->ISR & I2C_ISR_STOPF)) {
        if (_check_ack_status(i2c) != I2C_STATUS_OK) {
            return I2C_STATUS_ERROR;
        }
    }
    return I2C_STATUS_OK;
}

static inline uint8_t _wait_for_txis(I2C_TypeDef *i2c)
{
    while (!(i2c->ISR & I2C_ISR_TXIS)) {
        if (_check_ack_status(i2c) != I2C_STATUS_OK) {
            return I2C_STATUS_ERROR;
        }
    }
    return I2C_STATUS_OK;
}

static inline uint8_t _wait_for_rxne(I2C_TypeDef *i2c)
{
    while (!(i2c->ISR & I2C_ISR_RXNE)) {
        if (_check_ack_status(i2c) != I2C_STATUS_OK) {
            return I2C_STATUS_ERROR;
        }
    }
    return I2C_STATUS_OK;
}

int i2c_acquire(i2c_t dev)
{
    if (dev >= I2C_NUMOF) {
        return -1;
    }
    mutex_lock(&locks[dev]);
    return 0;
}

int i2c_release(i2c_t dev)
{
    if (dev >= I2C_NUMOF) {
        return -1;
    }
    mutex_unlock(&locks[dev]);
    return 0;
}

int i2c_read_byte(i2c_t dev, uint8_t address, void *data)
{
    return i2c_read_bytes(dev, address, data, 1);
}

int i2c_read_bytes(i2c_t dev, uint8_t address, void *data, int length)
{
    int xfer_count = length;
    int xfer_size;
    uint8_t *xfer_ptr = data;

    if ((unsigned int)dev >= I2C_NUMOF) {
        return -1;
    }

    I2C_TypeDef *i2c = i2c_config[dev].dev;

    /* wait till bus not busy */
    while (i2c->ISR & I2C_ISR_BUSY) {}

    /* send slave address
     * if data is more than can be read in one go, use reload mode
     * if not, use autoend mode */
    if (xfer_count > MAX_NBYTE_SIZE) {
        xfer_size = MAX_NBYTE_SIZE;
        _i2c_transfer_config(i2c, address, xfer_size, I2C_RELOAD_MODE, I2C_GENERATE_START_READ);
    }
    else {
        xfer_size = xfer_count;
        _i2c_transfer_config(i2c, address, xfer_size, I2C_AUTOEND_MODE, I2C_GENERATE_START_READ);
    }

    /* receive data */
    while (xfer_count > 0) {
        /* wait until RXNE flag is set (means a byte was received) */
        if (_wait_for_rxne(i2c) != I2C_STATUS_OK) {
            return -2;
        }
        /* read data from RXDR */
        *(xfer_ptr++) = i2c->RXDR;
        xfer_size--;
        xfer_count--;

        if ((xfer_size == 0) && (xfer_count != 0)) {
            /* wait until TCR (transfer complete reload) flag is set */
            while (!(i2c->ISR & I2C_ISR_TCR)) {}
            if (xfer_count > MAX_NBYTE_SIZE) {
                xfer_size = MAX_NBYTE_SIZE;
                _i2c_transfer_config(i2c, address, xfer_size, I2C_RELOAD_MODE, I2C_NO_STARTSTOP);
            }
            else {
                xfer_size = xfer_count;
                _i2c_transfer_config(i2c, address, xfer_size, I2C_AUTOEND_MODE, I2C_NO_STARTSTOP);
            }
        }
    }

    /* no need to check TC flag for completion, in AUTOEND mode the stop is auto generated */
    if (_wait_for_stop(i2c) != I2C_STATUS_OK) {
        return -2;
    }

    /* clear STOPF flag */
    i2c->ICR = I2C_ICR_STOPCF;

    /* clear CR2 register */
    _clear_cr2(i2c);

    return length;
}

int i2c_read_reg(i2c_t dev, uint8_t address, uint8_t reg, void *data)
{
    return i2c_read_regs(dev, address, reg, data, 1);
}

int i2c_read_regs(i2c_t dev, uint8_t address, uint8_t reg, void *data, int length)
{
    int xfer_count = length;
    int xfer_size;
    uint8_t *xfer_ptr = data;

    if ((unsigned int)dev >= I2C_NUMOF) {
        return -1;
    }

    I2C_TypeDef *i2c = i2c_config[dev].dev;

    /* wait till bus not busy */
    while (i2c->ISR & I2C_ISR_BUSY) {}
    /* init write */
    _i2c_transfer_config(i2c, address, 1, I2C_SOFTEND_MODE, I2C_GENERATE_START_WRITE);
    /* wait until TXIS flag is set (means ready to send) */
    if (_wait_for_txis(i2c) != I2C_STATUS_OK) {
        return -2;
    }
    /* write reg address */
    i2c->TXDR = reg;
    /* wait until TC flag is set */
    while (!(i2c->ISR & I2C_ISR_TC)) {}

    /* send slave address */
    /* set NBYTES to write and reload if xfer_count > MAX_NBYTE_SIZE and generate RESTART */
    if (xfer_count > MAX_NBYTE_SIZE) {
        xfer_size = MAX_NBYTE_SIZE;
        _i2c_transfer_config(i2c, address, xfer_size, I2C_RELOAD_MODE, I2C_GENERATE_START_READ);
    }
    else {
        xfer_size = xfer_count;
        _i2c_transfer_config(i2c, address, xfer_size, I2C_AUTOEND_MODE, I2C_GENERATE_START_READ);
    }

    do {
        /* wait until RXNE flag is set (means a byte was received) */
        if (_wait_for_rxne(i2c) != I2C_STATUS_OK) {
            return -2;
        }
        /* read data from RXDR */
        *(xfer_ptr++) = i2c->RXDR;
        xfer_size--;
        xfer_count--;

        if ((xfer_size == 0) && (xfer_count != 0)) {
            /* wait until TCR (transfer complete reload) flag is set */
            while (!(i2c->ISR & I2C_ISR_TCR)) {}
            if (xfer_count > MAX_NBYTE_SIZE) {
                xfer_size = MAX_NBYTE_SIZE;
                _i2c_transfer_config(i2c, address, xfer_size, I2C_RELOAD_MODE, I2C_NO_STARTSTOP);
            }
            else {
                xfer_size = xfer_count;
                _i2c_transfer_config(i2c, address, xfer_size, I2C_AUTOEND_MODE, I2C_NO_STARTSTOP);
            }
        }
    } while (xfer_count > 0);

    /* no need to check TC flag for completion, in AUTOEND mode the stop is auto generated */
    if (_wait_for_stop(i2c) != I2C_STATUS_OK) {
        return -2;
    }

    /* clear STOPF flag */
    i2c->ICR = I2C_ICR_STOPCF;

    /* clear CR2 register */
    _clear_cr2(i2c);

    return length;
}

int i2c_write_byte(i2c_t dev, uint8_t address, uint8_t data)
{
    return i2c_write_bytes(dev, address, &data, 1);
}

int i2c_write_bytes(i2c_t dev, uint8_t address, const void *data, int length)
{
    int xfer_count = length;
    int xfer_size;
    uint8_t *xfer_ptr = data; /* cuz compiler nags me */

    if ((unsigned int)dev >= I2C_NUMOF) {
        return -1;
    }

    I2C_TypeDef *i2c = i2c_config[dev].dev;

    /* wait till bus not busy */
    while (i2c->ISR & I2C_ISR_BUSY) {}

    /* send slave address */
    /* set NBYTES to write and reload if xfer_count > MAX_NBYTE_SIZE and generate START */
    if (xfer_count > MAX_NBYTE_SIZE) {
        xfer_size = MAX_NBYTE_SIZE;
        _i2c_transfer_config(i2c, address, xfer_size, I2C_RELOAD_MODE, I2C_GENERATE_START_WRITE);
    }
    else {
        xfer_size = xfer_count;
        _i2c_transfer_config(i2c, address, xfer_size, I2C_AUTOEND_MODE, I2C_GENERATE_START_WRITE);
    }

    while (xfer_count > 0) {
        /* wait until TXIS flag is set (means ready to send) */
        if (_wait_for_txis(i2c) != I2C_STATUS_OK) {
            return -2;
        }
        /* write data to TXDR */
        i2c->TXDR = *(xfer_ptr++);
        xfer_count--;
        xfer_size--;

        if ((xfer_size == 0) && (xfer_count != 0)) {
            /* wait until TCR (transfer complete reload) flag is set */
            while (!(i2c->ISR & I2C_ISR_TCR)) {}
            if (xfer_count > MAX_NBYTE_SIZE) {
                xfer_size = MAX_NBYTE_SIZE;
                _i2c_transfer_config(i2c, address, xfer_size, I2C_RELOAD_MODE, I2C_NO_STARTSTOP);
            }
            else {
                xfer_size = xfer_count;
                _i2c_transfer_config(i2c, address, xfer_size, I2C_AUTOEND_MODE, I2C_NO_STARTSTOP);
            }
        }
    }

    /* no need to check TC flag for completion, in AUTOEND mode the stop is auto generated */
    if (_wait_for_stop(i2c) != I2C_STATUS_OK) {
        return -2;
    }

    /* clear STOPF flag */
    i2c->ICR = I2C_ICR_STOPCF;

    /* clear CR2 register */
    _clear_cr2(i2c);

    return length;
}

int i2c_write_reg(i2c_t dev, uint8_t address, uint8_t reg, uint8_t data)
{
    return i2c_write_regs(dev, address, reg, &data, 1);
}

int i2c_write_regs(i2c_t dev, uint8_t address, uint8_t reg, const void *data, int length)
{
    int xfer_count = length;
    int xfer_size;
    uint8_t *xfer_ptr = data;

    if ((unsigned int)dev >= I2C_NUMOF) {
        return -1;
    }

    I2C_TypeDef *i2c = i2c_config[dev].dev;

    /* wait till bus not busy */
    while (i2c->ISR & I2C_ISR_BUSY) {}

    /* send slave address+START and register address */
    _i2c_transfer_config(i2c, address, 1, I2C_RELOAD_MODE, I2C_GENERATE_START_WRITE);
    /* wait until TXIS flag is set (means ready to send) */
    if (_wait_for_txis(i2c) != I2C_STATUS_OK) {
        return -2;
    }
    /* write reg address */
    i2c->TXDR = reg;
    /* wait until TCR (transfer complete reload) flag is set */
    while (!(i2c->ISR & I2C_ISR_TCR)) {}

    /* send data */
    /* set NBYTES to write and reload if xfer_count > MAX_NBYTE_SIZE */
    if (xfer_count > MAX_NBYTE_SIZE) {
        xfer_size = MAX_NBYTE_SIZE;
        _i2c_transfer_config(i2c, address, xfer_size, I2C_RELOAD_MODE, I2C_NO_STARTSTOP);
    }
    else {
        xfer_size = xfer_count;
        _i2c_transfer_config(i2c, address, xfer_size, I2C_AUTOEND_MODE, I2C_NO_STARTSTOP);
    }

    while (xfer_count > 0) {
        /* wait until TXIS flag is set (means ready to send) */
        if (_wait_for_txis(i2c) != I2C_STATUS_OK) {
            return -2;
        }
        /* write data to TXDR */
        i2c->TXDR = *(xfer_ptr++);
        xfer_count--;
        xfer_size--;

        if ((xfer_size == 0) && (xfer_count != 0)) {
            /* wait until TCR (transfer complete reload) flag is set */
            while (!(i2c->ISR & I2C_ISR_TCR)) {}
            if (xfer_count > MAX_NBYTE_SIZE) {
                xfer_size = MAX_NBYTE_SIZE;
                _i2c_transfer_config(i2c, address, xfer_size, I2C_RELOAD_MODE, I2C_NO_STARTSTOP);
            }
            else {
                xfer_size = xfer_count;
                _i2c_transfer_config(i2c, address, xfer_size, I2C_AUTOEND_MODE, I2C_NO_STARTSTOP);
            }
        }
    }

    /* no need to check TC flag for completion, in AUTOEND mode the stop is auto generated */
    if (_wait_for_stop(i2c) != I2C_STATUS_OK) {
        return -2;
    }

    /* clear STOPF flag */
    i2c->ICR = I2C_ICR_STOPCF;

    /* clear CR2 register */
    _clear_cr2(i2c);

    return length;
}

void i2c_poweron(i2c_t dev)
{
    if ((unsigned int)dev < I2C_NUMOF) {
        periph_clk_en(APB1, (RCC_APB1ENR1_I2C1EN << dev));
    }
}

void i2c_poweroff(i2c_t dev)
{
    if ((unsigned int)dev < I2C_NUMOF) {
        while (i2c_config[dev].dev->ISR & I2C_ISR_BUSY) {}
        periph_clk_dis(APB1, (RCC_APB1ENR1_I2C1EN << dev));
    }
}

#if I2C_0_EN
void I2C_0_ERR_ISR(void)
{
    unsigned state = I2C1->ISR;
    DEBUG("\n\n### I2C1 ERROR OCCURED ###\n");
    DEBUG("status: %08x\n", state);
    if (state & I2C_ISR_OVR) {
        DEBUG("OVR\n");
    }
    if (state & I2C_ISR_ARLO) {
        DEBUG("ARLO\n");
    }
    if (state & I2C_ISR_BERR) {
        DEBUG("BERR\n");
    }
    if (state & I2C_ISR_PECERR) {
        DEBUG("PECERR\n");
    }
    if (state & I2C_ISR_TIMEOUT) {
        DEBUG("TIMEOUT\n");
    }
    if (state & I2C_ISR_ALERT) {
        DEBUG("ALERT\n");
    }
    while (1) {}
}
#endif /* I2C_0_EN */

#if I2C_1_EN
void I2C_1_ERR_ISR(void)
{
    unsigned state = I2C2->ISR;
    DEBUG("\n\n### I2C2 ERROR OCCURED ###\n");
    DEBUG("status: %08x\n", state);
    if (state & I2C_ISR_OVR) {
        DEBUG("OVR\n");
    }
    if (state & I2C_ISR_ARLO) {
        DEBUG("ARLO\n");
    }
    if (state & I2C_ISR_BERR) {
        DEBUG("BERR\n");
    }
    if (state & I2C_ISR_PECERR) {
        DEBUG("PECERR\n");
    }
    if (state & I2C_ISR_TIMEOUT) {
        DEBUG("TIMEOUT\n");
    }
    if (state & I2C_ISR_ALERT) {
        DEBUG("ALERT\n");
    }
    while (1) {}
}
#endif /* I2C_1_EN */

#endif /* I2C_NUMOF */
