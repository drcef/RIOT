/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     drivers_pca9554
 * @{
 *
 * @file
 * @brief       Register definition for the PCA9554 I/O Expander driver
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 *
 */

#ifndef PCA9554_REG_H
#define PCA9554_REG_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief   Device commands
 * @{
 */
#define PCA9554_INPUT_PORT_REG 	 	0x00 /**< Input Port register */
#define PCA9554_OUTPUT_PORT_REG		0x01 /**< Output Port register */
#define PCA9554_POLARITY_INV_REG	0x02 /**< Polarity Inversion register */
#define PCA9554_CONFIG_REG			0x03 /**< Configuration (direction) register */
/** @} */

#ifdef __cplusplus
}
#endif

#endif /** @} */
