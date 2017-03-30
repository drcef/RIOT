/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_mcp9808
 * @{
 *
 * @file
 * @brief       Register definition for the MCP9808 sensor driver
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 *
 */

#ifndef MCP9808_REG_H
#define MCP9808_REG_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief   Device registers
 * @{
 */
#define MCP9808_REG_CONFIGURATION      0x01
#define MCP9808_REG_UPPER_TEMP         0x02
#define MCP9808_REG_LOWER_TEMP         0x03
#define MCP9808_REG_CRIT_TEMP          0x04
#define MCP9808_REG_AMBIENT_TEMP       0x05
#define MCP9808_REG_MANUF_ID           0x06
#define MCP9808_REG_DEVICE_ID          0x07
#define MCP9808_REG_RESOLUTION         0x08

/** @} */

/**
 * @brief   Configuration register masks
 * @{
 */
#define MCP9808_REG_CONFIG_HYSTERISIS  0x0600
#define MCP9808_REG_CONFIG_SHUTDOWN    0x0100
#define MCP9808_REG_CONFIG_CRITLOCKED  0x0080
#define MCP9808_REG_CONFIG_WINLOCKED   0x0040
#define MCP9808_REG_CONFIG_INTCLR      0x0020
#define MCP9808_REG_CONFIG_ALERTSTAT   0x0010
#define MCP9808_REG_CONFIG_ALERTCTRL   0x0008
#define MCP9808_REG_CONFIG_ALERTSEL    0x0004
#define MCP9808_REG_CONFIG_ALERTPOL    0x0002
#define MCP9808_REG_CONFIG_ALERTMODE   0x0001
/** @} */

#ifdef __cplusplus
}
#endif

#endif /** @} */
