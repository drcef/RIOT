/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 *
 */

/**
 * @ingroup     drivers_max17055
 * @{
 *
 * @file
 * @brief       Register definition for the MAX17055 fuel gauge driver
 *
 * @author      George Psimenos <gp7g14@soton.ac.uk>
 *
 */

#ifndef MAX17055_REGS_H
#define MAX17055_REGS_H

#ifdef __cplusplus
extern "C"
{
#endif


/**
 * @name    Register definitions
 * @{
 */
#define MAX17055_REG_STATUS			0x00
#define MAX17055_REG_VALRTTH		0x01
#define MAX17055_REG_TALRTTH		0x02
#define MAX17055_REG_SALRTTH		0x03
#define MAX17055_REG_ATRATE			0x04
#define MAX17055_REG_REPCAP			0x05
#define MAX17055_REG_REPSOC			0x06
#define MAX17055_REG_AGE			0x07
#define MAX17055_REG_TEMP			0x08
#define MAX17055_REG_VCELL			0x09
#define MAX17055_REG_CURRENT		0x0A
#define MAX17055_REG_AVGCURRENT		0x0B
#define MAX17055_REG_QRESIDUAL		0x0C
#define MAX17055_REG_MIXSOC			0x0D
#define MAX17055_REG_AVSOC			0x0E
#define MAX17055_REG_MIXCAP			0x0F

#define MAX17055_REG_FULLCAPREP		0x10
#define MAX17055_REG_TTE			0x11
#define MAX17055_REG_QRTABLE00		0x12
#define MAX17055_REG_FULLSOCTHR		0x13
#define MAX17055_REG_RCELL			0x14
#define MAX17055_REG_AVGTA			0x16
#define MAX17055_REG_CYCLES			0x17
#define MAX17055_REG_DESIGNCAP		0x18
#define MAX17055_REG_AVGVCELL		0x19
#define MAX17055_REG_MAXMINTEMP		0x1A
#define MAX17055_REG_MAXMINVOLT		0x1B
#define MAX17055_REG_MAXMINCURR		0x1C
#define MAX17055_REG_CONFIG			0x1D
#define MAX17055_REG_ICHGTERM		0x1E
#define MAX17055_REG_AVCAP			0x1F
#define MAX17055_REG_TTF			0x20

#define MAX17055_REG_DEVNAME		0x21
#define MAX17055_REG_QRTABLE10		0x22
#define MAX17055_REG_FULLCAPNOM		0x23
#define MAX17055_REG_AIN			0x27
#define MAX17055_REG_LEARNCGF		0x28
#define MAX17055_REG_FILTERCFG		0x29
#define MAX17055_REG_RELAXCFG		0x2A
#define MAX17055_REG_MISCCFG		0x2B
#define MAX17055_REG_TGAIN			0x2C
#define MAX17055_REG_TOFF			0x2D
#define MAX17055_REG_CGAIN			0x2E
#define MAX17055_REG_COFF			0x2F

#define MAX17055_REG_QRTABLE20		0x32
#define MAX17055_REG_DIETEMP		0x34
#define MAX17055_REG_FULLCAP		0x35
#define MAX17055_REG_RCOMP0			0x38
#define MAX17055_REG_TEMPCO			0x39
#define MAX17055_REG_VEMPTY			0x3A
#define MAX17055_REG_FSTAT			0x3D
#define MAX17055_REG_TIMER			0x3E
#define MAX17055_REG_SHDNTIMER		0x3F

#define MAX17055_REG_USERMEM1		0x40
#define MAX17055_REG_QRTABLE30		0x42
#define MAX17055_REG_RGAIN			0x43
#define MAX17055_REG_DQACC			0x45
#define MAX17055_REG_DPACC			0x46
#define MAX17055_REG_CONVGCFG		0x49
#define MAX17055_REG_VFREMCAP		0x4A
#define MAX17055_REG_QH				0x4D

#define MAX17055_REG_SOFT_WAKEUP	0x60

#define MAX17055_REG_STATUS2		0xB0
#define MAX17055_REG_POWER			0xB1
#define MAX17055_REG_ID				0xB2
#define MAX17055_REG_AVGPOWER		0xB3
#define MAX17055_REG_IALRTTH		0xB4
#define MAX17055_REG_CVMIXCAP		0xB6
#define MAX17055_REG_CVHALFTIME		0xB7
#define MAX17055_REG_CGTEMPCO		0xB8
#define MAX17055_REG_CURVE			0xB9
#define MAX17055_REG_HIBCFG			0xBA
#define MAX17055_REG_CONFIG2		0xBB
#define MAX17055_REG_VRIPPLE		0xBC
#define MAX17055_REG_RIPPLECFG		0xBD
#define MAX17055_REG_TIMERH			0xBE

#define MAX17055_REG_RSENSE			0xD0
#define MAX17055_REG_SCOCVLIM		0xD1
#define MAX17055_REG_SOCHOLD		0xD3
#define MAX17055_REG_MAXPEAKPOWER	0xD4
#define MAX17055_REG_SUSPEAKPOWER	0xD5
#define MAX17055_REG_PACKRESISTANCE	0xD6
#define MAX17055_REG_SYSRESISTANCE	0xD7
#define MAX17055_REG_MINSYSVOLTAGE	0xD8
#define MAX17055_REG_MPPCURRENT		0xD9
#define MAX17055_REG_SPPCURRENT		0xDA
#define MAX17055_REG_MODELCFG		0xDB
#define MAX17055_REG_ATQRESIDUAL	0xDC
#define MAX17055_REG_ATTTE			0xDD
#define MAX17055_REG_ATAVSOC		0xDE
#define MAX17055_REG_ATAVCAP		0xDF
/** @} */

/**
 * @name    Register mask definitions
 * @{
 */
#define MAX17055_STATUS_POR			0x0002

#define MAX17055_FSTAT_DNR			0x0001

#define MAX17055_HIBCFG_ENHIB		0x8000

#define MAX17055_MODELCFG_REFRESH	0x8000

#define MAX17055_SOFT_WAKEUP_CLEAR	0x0000
#define MAX17055_SOFT_WAKEUP_SET	0x0090
/** @} */

/**
 * @name    Magic values (refer to datasheet)
 * @{
 */
#define MAX17055_HIBCFG_VAL			0x070C
/** @} */

#ifdef __cplusplus
}
#endif

#endif /* MAX17055_REGS_H */
/** @} */
