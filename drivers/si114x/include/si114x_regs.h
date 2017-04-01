/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_si114x
 * @{
 *
 * @file
 * @brief       Register definition for the SI114X sensor driver
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 *
 */

#ifndef SI114X_REG_H
#define SI114X_REG_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief   Device registers
 * @{
 */
#define SI114X_REG_PART_ID              0x00
#define SI114X_REG_REV_ID               0x01
#define SI114X_REG_SEQ_ID               0x02
#define SI114X_REG_INT_CFG              0x03
#define SI114X_REG_IRQ_ENABLE           0x04
#define SI114X_REG_HW_KEY               0x07
#define SI114X_REG_MEAS_RATE0           0x08
#define SI114X_REG_MEAS_RATE1           0x09
#define SI114X_REG_PS_LED21             0x0F
#define SI114X_REG_PS_LED3              0x10
#define SI114X_REG_UCOEF0               0x13
#define SI114X_REG_UCOEF1               0x14
#define SI114X_REG_UCOEF2               0x15
#define SI114X_REG_UCOEF3               0x16
#define SI114X_REG_PARAM_WR             0x17
#define SI114X_REG_COMMAND              0x18
#define SI114X_REG_RESPONSE             0x20
#define SI114X_REG_IRQ_STATUS           0x21
#define SI114X_REG_ALS_VIS_DATA0        0x22
#define SI114X_REG_ALS_VIS_DATA1        0x23
#define SI114X_REG_ALS_IR_DATA0         0x24
#define SI114X_REG_ALS_IR_DATA1         0x25
#define SI114X_REG_PS1_DATA0            0x26
#define SI114X_REG_PS1_DATA1            0x27
#define SI114X_REG_PS2_DATA0            0x28
#define SI114X_REG_PS2_DATA1            0x29
#define SI114X_REG_PS3_DATA0            0x2A
#define SI114X_REG_PS3_DATA1            0x2B
#define SI114X_REG_UVINDEX0             0x2C
#define SI114X_REG_UVINDEX1             0x2D
#define SI114X_REG_PARAM_RD             0x2E
#define SI114X_REG_CHIP_STAT            0x30
/** @} */

/**
 * @brief   Device commands
 * @{
 */
#define SI114X_CMD_PARAM_QUERY          0x80
#define SI114X_CMD_PARAM_SET            0xA0
#define SI114X_CMD_NOP                  0x00
#define SI114X_CMD_RESET                0x01
#define SI114X_CMD_BUSADDR              0x02
#define SI114X_CMD_PS_FORCE             0x05
#define SI114X_CMD_GET_CAL              0x12
#define SI114X_CMD_ALS_FORCE            0x06
#define SI114X_CMD_PSALS_FORCE          0x07
#define SI114X_CMD_PS_PAUSE             0x09
#define SI114X_CMD_ALS_PAUSE            0x0A
#define SI114X_CMD_PSALS_PAUSE          0x0B
#define SI114X_CMD_PS_AUTO              0x0D
#define SI114X_CMD_ALS_AUTO             0x0E
#define SI114X_CMD_PSALS_AUTO           0x0F
/** @} */

/**
 * @brief   Device parameters (in device RAM)
 * @{
 */
#define SI114X_RAM_I2C_ADDR             0x00
#define SI114X_RAM_CHLIST               0x01
#define SI114X_RAM_PSLED12_SELECT       0x02
#define SI114X_RAM_PSLED3_SELECT        0x03
#define SI114X_RAM_PS_ENCODING          0x05
#define SI114X_RAM_ALS_ENCODING         0x06
#define SI114X_RAM_PS1_ADCMUX           0x07
#define SI114X_RAM_PS2_ADCMUX           0x08
#define SI114X_RAM_PS3_ADCMUX           0x09
#define SI114X_RAM_PS_ADC_COUNTER       0x0A
#define SI114X_RAM_PS_ADC_GAIN          0x0B
#define SI114X_RAM_PS_ADC_MISC          0x0C
#define SI114X_RAM_ALS_IR_ADCMUX        0x0E
#define SI114X_RAM_AUX_ADCMUX           0x0F
#define SI114X_RAM_ALS_VIS_ADC_COUNTER  0x10
#define SI114X_RAM_ALS_VIS_ADC_GAIN     0x11
#define SI114X_RAM_ALS_VIS_ADC_MISC     0x12
#define SI114X_RAM_LED_REC              0x1C
#define SI114X_RAM_ALS_IR_ADC_COUNTER   0x1D
#define SI114X_RAM_ALS_IR_ADC_GAIN      0x1E
#define SI114X_RAM_ALS_IR_ADC_MISC      0x1F
/** @} */

/**
 * @brief   CHLIST bit masks
 * @{
 */
#define SI114X_CHLIST_EN_PS1            0x01
#define SI114X_CHLIST_EN_PS2            0x02
#define SI114X_CHLIST_EN_PS3            0x04
#define SI114X_CHLIST_EN_ALS_VIS        0x10
#define SI114X_CHLIST_EN_ALS_IR         0x20
#define SI114X_CHLIST_EN_AUX            0x40
#define SI114X_CHLIST_EN_UV             0x80
/** @} */

#ifdef __cplusplus
}
#endif

#endif /** @} */
