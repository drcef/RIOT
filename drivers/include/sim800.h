/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef SIM800_H
#define SIM800_H

#define UART_BUFSIZE        1024U

#include "periph/uart.h"
#include "ringbuffer.h"

typedef struct {
	uart_t uart;
	char rx_mem[UART_BUFSIZE];
	ringbuffer_t rx_buf;
	uint8_t lines_avail;
} sim800_t;

enum {
	SIM800_OK 				= 0,
	SIM800_READY 			= 1,
	SIM800_GPRS_READY		= 2,
	SIM800_UART_ERROR		= -1,
	SIM800_UART_NOBAUD		= -2,
	SIM800_UNRESPONSIVE		= -3,
	SIM800_FUNC_ERROR		= -4,
	SIM800_SIM_ERROR		= -5,
	SIM800_APN_ERROR		= -6,
	SIM800_GPRS_ERROR		= -7,
	SIM800_RSSI_ERROR		= -8,
	SIM800_POOR_SIGNAL		= -9,
	SIM800_REG_ERROR		= -10,
	SIM800_ATT_ERROR		= -11,
	SIM800_TCP_ERROR		= -12,
	SIM800_IP_ERROR			= -13,
	SIM800_TCP_NOCIPSTART	= -14,
	SIM800_TCP_NOCONNECT	= -15,
	SIM800_TCP_WRONGCONNECT	= -16,
	SIM800_TCP_NOSEND		= -17,
	SIM800_TCP_WRONGSEND	= -18,
	SIM800_BAUD_ERROR		= -19,
	SIM800_FORMAT_ERROR		= -20,
	SIM800_OPERATOR_ERROR	= -21,
	SIM800_NO_PROMPT		= -22,
	SIM800_NO_IP			= -23,
	SIM800_NO_OPERATOR		= -24,
	SIM800_IN_FLIGHT_MODE	= -25,
	SIM800_INVALID_MODE		= -26,
	SIM800_WRITE_ERROR		= -27,
	SIM800_OVERFLOW			= -28,
	SIM800_TCP_TIMEOUT		= -29,
	SIM800_ERASE_ERROR		= -30,
	SIM800_SEEK_ERROR		= -31,
	SIM800_SEQUENCE_OVF		= -32,
	SIM800_TCP_NOCIPCLOSE	= -33,
	SIM800_FILE_READ_ERROR	= -34,
};

enum {
	SIM800_WRITE_TO_RAM,
	SIM800_WRITE_TO_FILE,
	SIM800_READ_FROM_RAM,
	SIM800_READ_FROM_FILE,
};

int sim800_status(sim800_t *dev);
int sim800_rssi(sim800_t *dev);
int sim800_set_apn(sim800_t *dev, const char *apn, const char *user, const char *password);
int sim800_init(sim800_t *dev, uint8_t uart);
int sim800_operator_name(sim800_t *dev, char *name);
int sim800_attach(sim800_t *dev);
int sim800_detach(sim800_t *dev);
int sim800_gprs_connect(sim800_t *dev);
int sim800_gprs_disconnect(sim800_t *dev);
int sim800_powerdown(sim800_t *dev);
int sim800_http(sim800_t *dev,
	const char* host, uint16_t port,
	const char *req_head, void *req_body,
	char *res_head, void *res_body,
	int req_mode, int res_mode,
	int head_maxsize, int body_maxsize);

#endif /* SIM800_H */