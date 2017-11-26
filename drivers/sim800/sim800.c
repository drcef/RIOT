/*
 * Copyright (C) 2017 George Psimenos (gp7g14@soton.ac.uk)
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <string.h>

#include "sim800.h"
#include "periph/uart.h"
#include "ringbuffer.h"
#include "xtimer.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define SIM800_MSG_UART		1U
#define SIM800_MSG_CMD		2U

static void uart_cb(void *arg, uint8_t data)
{
	sim800_t *dev = (sim800_t*)arg;

	ringbuffer_add_one(&(dev->rx_buf), data);
	if (data == '\n') {
		dev->lines_avail++;
	}
}

static void _flush_uart(sim800_t *dev)
{
	ringbuffer_remove(&(dev->rx_buf), dev->rx_buf.avail);
	dev->lines_avail = 0;
}

static int _wait_for_prompt(sim800_t *dev, uint32_t timeout)
{
	uint8_t c;
	do {
		while (dev->rx_buf.avail > 0) {
			c = (uint8_t) ringbuffer_get_one(&(dev->rx_buf));
			if (c == '>') return 1;
		}
		xtimer_usleep(1000U);
	} while (--timeout);
	return 0;
}

static int _read_line(sim800_t *dev, uint8_t *rx, uint16_t size, uint32_t timeout)
{
	while(!dev->lines_avail) {
		xtimer_usleep(1000U);
		if (!--timeout) return -1;
	}

	uint16_t i = 0;
	int c = 0;
	do {
		c = ringbuffer_get_one(&(dev->rx_buf));
		if (c < 0) break; // ringbuffer empty
		*(rx+(i++)) = (uint8_t) c;
	} while ((c != '\n') && (i < size));
	if (c == '\n') dev->lines_avail--;
	rx[i] = 0;

	return i;
}

static int _cmd_resp_check(sim800_t *dev, const char *command, const char *response, int retries, uint32_t timeout, uint32_t interval)
{
	uint8_t rx[100];
	int len;

	for (int i=0; i<retries; i++) {
		_flush_uart(dev);
		uart_write(dev->uart, (uint8_t*)command, strlen(command));
		len = _read_line(dev, rx, sizeof(rx), timeout); // ignore echo
		DEBUG("echo %i: %s", len, rx);
		len = _read_line(dev, rx, sizeof(rx), timeout);
		if (len > 0) {
			DEBUG("read %i: %s", len, rx);
			if (strstr((char*)rx, response) != NULL) return 1;
		}
		DEBUG("retry...\n");
		xtimer_usleep(interval * 1000U);
	}

	return 0;
}

static int _cmd_read(sim800_t *dev, const char *command, char *response, uint16_t size, int retries, uint32_t timeout)
{
	int len;
	for (int i=0; i<retries; i++) {
		_flush_uart(dev);
		uart_write(dev->uart, (uint8_t*)command, strlen(command));
		len = _read_line(dev, (uint8_t*)response, size, timeout); // ignore echo
		DEBUG("echo %i: %s", len, response);
		len = _read_line(dev, (uint8_t*)response, size, timeout);
		if (len > 0) {
			DEBUG("read %i: %s", len, response);
			return len;
		}
		DEBUG("retry...\n");
	}

	return 0;
}

/*
static int _resp_wait(sim800_t *dev, const char *response, uint32_t timeout)
{
	uint8_t rx[100];

	_flush_uart(dev);
	if (_read_line(dev, rx, sizeof(rx), timeout) > 0) {
		if (strstr((char*)rx, response) != NULL) return 1;
	}

	return 0;
}
*/

static int _receive_tcp(sim800_t *dev, char *rx, uint16_t size, uint32_t timeout)
{
	uint8_t parser[8];
	uint8_t i = 0;
	uint8_t c = 0;
	int rx_count = 0;
	
	do {
		while (dev->rx_buf.avail > 0) {
			c = (uint8_t) ringbuffer_get_one(&(dev->rx_buf));
			rx[rx_count++] = (char)c;
			for (i=0; i<sizeof(parser)-1; i++) parser[i] = parser[i+1];
			parser[i] = c;
			if (strstr((char*)parser, "CLOSED") != NULL) return rx_count;
			if (rx_count >= size) return rx_count;
		}
		xtimer_usleep(1000U);
	} while(--timeout);

	return rx_count;
}

int sim800_status(sim800_t *dev)
{
	if (!_cmd_resp_check(dev, "AT\r\n", "OK", 5, 200, 1000)) return SIM800_UNRESPONSIVE;
	if (!_cmd_resp_check(dev, "AT+CFUN?\r\n", "+CFUN: 1", 5, 200, 1000)) return SIM800_FUNC_ERROR;
	if (!_cmd_resp_check(dev, "AT+CPIN?\r\n", "+CPIN: READY", 5, 200, 1000)) return SIM800_SIM_ERROR;
	if (!_cmd_resp_check(dev, "AT+CREG?\r\n", "+CREG: 0,1", 5, 200, 2000)) return SIM800_REG_ERROR;
	//if (!_cmd_resp_check(dev, "AT+CGATT?\r\n", "+CGATT: 1", 5, 200, 2000)) return SIM800_ATT_ERROR;

	return SIM800_READY;
}

int sim800_rssi(sim800_t *dev)
{
	char rssi_str[20];
	unsigned int rssi = 0;
	unsigned int ber = 0;

	if (!_cmd_read(dev, "AT+CSQ\r\n", rssi_str, sizeof(rssi_str), 3, 200)) return SIM800_UNRESPONSIVE;
	if (sscanf(rssi_str, "+CSQ: %u,%u", &rssi, &ber) != 2) return SIM800_RSSI_ERROR;
	if (rssi == 99) return SIM800_RSSI_ERROR;

	return rssi;
}

int sim800_set_apn(sim800_t *dev, const char *apn, const char *user, const char *password)
{
	char apn_str[100];
	sprintf(apn_str, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", apn, user, password);

	if (!_cmd_resp_check(dev, apn_str, "OK", 5, 200, 1000))
		//return SIM800_APN_ERROR;
		return SIM800_OK;
	return SIM800_OK;
}

int sim800_init(sim800_t *dev, uint8_t uart)
{
	int res;
	int status;

	dev->uart = UART_DEV(uart);
	dev->lines_avail = 0;

	ringbuffer_init(&(dev->rx_buf), dev->rx_mem, UART_BUFSIZE);

	res = uart_init(dev->uart, 115200U, uart_cb, (void *)dev);
	if (res == UART_NOBAUD) return SIM800_UART_NOBAUD;
	else if (res != UART_OK) return SIM800_UART_ERROR;

	status = sim800_status(dev);
	if (status != SIM800_READY) return status;

	if (sim800_set_apn(dev, "pp.vodafone.co.uk", "web", "web") != SIM800_OK) return SIM800_APN_ERROR;

	return SIM800_READY;
}

int sim800_gprs_connect(sim800_t *dev)
{
	char ip_addr_str[24];
	unsigned int ip0 = 0, ip1 = 0, ip2 = 0, ip3 = 0;

	int signal_strength = sim800_rssi(dev);
	xtimer_usleep(5 * 1000U); // wait a bit after sim800_rssi()
	if (signal_strength < 5 || signal_strength == SIM800_RSSI_ERROR) return SIM800_POOR_SIGNAL;

	if (!_cmd_resp_check(dev, "AT+CIICR\r\n", "OK", 2, 20000, 10000)) return SIM800_GPRS_ERROR;
	if (!_cmd_read(dev, "AT+CIFSR\r\n", ip_addr_str, sizeof(ip_addr_str), 3, 300)) return SIM800_UNRESPONSIVE;
	
	if (sscanf(ip_addr_str, "%u.%u.%u.%u", &ip0, &ip1, &ip2, &ip3) != 4) return SIM800_IP_ERROR;

	return SIM800_GPRS_READY;
}

int sim800_gprs_disconnect(sim800_t *dev)
{
	if (!_cmd_resp_check(dev, "AT+CIPSHUT\r\n", "SHUT OK", 3, 1000, 2000)) return SIM800_UNRESPONSIVE;

	return SIM800_READY;
}

int sim800_http(sim800_t *dev, const char* host, uint16_t port, const char *request, char *response, uint16_t size)
{
	char rx[100];
	char cipstart_str[100];
	uint8_t terminate = 0x1a;
	uint16_t response_len;

	sprintf(cipstart_str, "AT+CIPSTART=\"TCP\",\"%s\",\"%u\"\r\n", host, port);
	if (!_cmd_resp_check(dev, cipstart_str, "OK", 3, 500, 1000)) { DEBUG("CIPSTART ERROR\n"); return SIM800_TCP_ERROR; }

	// now wait for connection to be established!
	// the modem sends an empty line before CONNECT OK
	// we read_line that first, and then read_line again to get CONNECT_OK
	// this should all be replaced by a parser that keeps comparing the last 10 received bytes against the desired value
	// also for later on where we wait for SEND OK
	if (_read_line(dev, (uint8_t*)rx, sizeof(rx), 10000) <= 0) { DEBUG("NO CONNECT OK\n"); return SIM800_TCP_ERROR; }
	_read_line(dev, (uint8_t*)rx, sizeof(rx), 200);
	if (strstr(rx, "CONNECT OK") == NULL) { DEBUG("WRONG CONNECT\n"); return SIM800_TCP_ERROR; }

	// now we can send data!
	uart_write(dev->uart, (const uint8_t*)"AT+CIPSEND\r\n", strlen("AT+CIPSEND\r\n"));
	if (!_wait_for_prompt(dev, 5000)) { DEBUG("NO PROMPT\n"); return SIM800_UNRESPONSIVE; }
	uart_write(dev->uart, (uint8_t*)request, strlen(request));
	uart_write(dev->uart, &terminate, sizeof(terminate));


	// now wait for confirmation!
	// flush the shit that's been echoed back first
	xtimer_usleep(12 * 1000U);
	_flush_uart(dev);
	if (_read_line(dev, (uint8_t*)rx, sizeof(rx), 10000) <= 0) { DEBUG("NO TCP ACK\n"); return SIM800_TCP_ERROR; }
	_read_line(dev, (uint8_t*)rx, sizeof(rx), 200);
	if (strstr(rx, "SEND OK") == NULL) { DEBUG("WRONG TCP ACK\n"); return SIM800_TCP_ERROR; }

	DEBUG("reading http response...\n");
	// get response!
	response_len = _receive_tcp(dev, response, size, 20000);
	if (response_len < sizeof(response)) response[response_len] = 0; // terminate
	else response[response_len-1] = 0; // terminate, losing the last char, but not overflowing

	return SIM800_OK;
}

/*
int sim800_http_proper(sim800_t *dev, const char* host, uint16_t port, const char *request, char *response, uint16_t size)
{
	char rx[100];
	char cipstart_str[100];
	uint8_t terminate = 0x1a;
	uint16_t response_len;

	if (!_cmd_resp_check(dev, "AT+HTTPINIT\r\n", "OK", 3, 1000, 2000)) { DEBUG("HTTPINIT ERROR\n"); return SIM800_TCP_ERROR; }


}
*/