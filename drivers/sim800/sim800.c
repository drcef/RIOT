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
#include "vfs.h"

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

// only call this BEFORE you write a command
// it will block until the UART receive buffer is empty
// AND until no characters have been received in the last millisecond
static void _flush_uart(sim800_t *dev)
{
	// keep emptying the ringbuffer until it stays empty
	// this could occur if we're receiving data right now
	// *** could this function pose a race condition against uart_cb?

	do {
		dev->lines_avail = 0;
		ringbuffer_remove(&(dev->rx_buf), dev->rx_buf.avail);
		xtimer_usleep(1000U);
	} while (dev->rx_buf.avail > 0);
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

// todo: optimize so that it doesn't wait for <interval> after last failed attempt
static int _cmd_resp_check(sim800_t *dev, const char *command, const char *response, int retries, uint32_t timeout, uint32_t interval)
{
	uint8_t rx[100];
	int len;

	for (int i=0; i<retries; i++) {
		_flush_uart(dev); // flush stuff received earlier
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

static int _receive_tcp(sim800_t *dev, char *rx, uint16_t size, const char *end_sequence, uint32_t timeout)
{
	char c = 0;
	int i = 0;
	int rx_count = 0;

	char parser[20];
	memset(parser, 0, sizeof(parser));

	int seq_len = strlen(end_sequence);
	if (seq_len > sizeof(parser)-1) return SIM800_SEQUENCE_OVF;
	
	do {
		while (dev->rx_buf.avail > 0) {
			c = (char) ringbuffer_get_one(&(dev->rx_buf));
			rx[rx_count++] = c;

			if (rx_count >= size) return SIM800_OVERFLOW;

			for (i=0; i<seq_len-1; i++) parser[i] = parser[i+1];
			parser[i] = c;
			if (strcmp(parser, end_sequence) == 0) {
				// terminate before end sequence
				rx[rx_count - seq_len] = 0;
				return rx_count;
			}
		}
		xtimer_usleep(1000U);
	} while(--timeout);
	return SIM800_TCP_TIMEOUT;
}

static int _receive_tcp_file(sim800_t *dev, int fd, uint16_t size, const char *end_sequence, uint32_t timeout)
{
	char page_buf[256];
	char c = 0;
	int i = 0;
	int rx_count = 0;
	int part_count = 0;
	int res;

	char parser[20];
	memset(parser, 0, sizeof(parser));

	int seq_len = strlen(end_sequence);
	if (seq_len > sizeof(parser)-1) return SIM800_SEQUENCE_OVF;
	
	do {
		while (dev->rx_buf.avail > 0) {
			// read one byte into page buffer
			rx_count++;
			c = (char) ringbuffer_get_one(&(dev->rx_buf));
			page_buf[part_count++] = c;

			// write to file when page buffer is full
			if (part_count >= sizeof(page_buf)) {
				res = vfs_write(fd, page_buf, part_count);
				if (res < part_count) return SIM800_WRITE_ERROR;
				part_count = 0;
			}
			
			if (rx_count >= size) return SIM800_OVERFLOW;

			for (i=0; i<seq_len-1; i++) parser[i] = parser[i+1];
			parser[i] = c;
			if (strcmp(parser, end_sequence) == 0) goto end;
		}
		xtimer_usleep(1000U);
	} while(--timeout);
	return SIM800_TCP_TIMEOUT;

end:
	// write remaining bytes to file
	if (part_count > seq_len) {
		part_count -= seq_len;
		res = vfs_write(fd, page_buf, part_count);
		if (res < part_count) return SIM800_WRITE_ERROR;
	}
	else if (part_count < seq_len) {
		// uh oh, part or all of the end sequence was written to the file
		// rewind the file and overwrite the end sequence with zeroes
		int erase_count = seq_len - part_count;
		res = vfs_lseek(fd, -erase_count, SEEK_END);
		if (res < 0) return SIM800_SEEK_ERROR;
		memset(parser, 0, erase_count);
		res = vfs_write(fd, parser, erase_count);
		if (res < erase_count) return SIM800_ERASE_ERROR;
	}

	return rx_count;
}

static int _wait_for_sequence(sim800_t *dev, const char *sequence, uint32_t timeout)
{
	int i = 0;
	int c = 0;

	char parser[20];
	memset(parser, 0, sizeof(parser));

	int seq_len = strlen(sequence);
	if (seq_len > sizeof(parser)-1) return SIM800_SEQUENCE_OVF;
	
	do {
		while (dev->rx_buf.avail > 0) {
			c = (char) ringbuffer_get_one(&(dev->rx_buf));
			for (i=0; i<seq_len-1; i++) parser[i] = parser[i+1];
			parser[i] = c;
			if (strcmp(parser, sequence) == 0) return seq_len;
		}
		xtimer_usleep(1000U);
	} while(--timeout);
	return SIM800_TCP_TIMEOUT;
}

int sim800_status(sim800_t *dev)
{
	if (!_cmd_resp_check(dev, "AT\r\n", "OK", 10, 250, 250)) return SIM800_UNRESPONSIVE;

	// set baud rate
	// *** todo: this isn't something to logically be in the status function
	if (!_cmd_resp_check(dev, "AT+IPR=115200\r\n", "OK", 4, 250, 250)) return SIM800_BAUD_ERROR;

	if (!_cmd_resp_check(dev, "AT+CFUN?\r\n", "+CFUN: 1", 10, 200, 500)) return SIM800_FUNC_ERROR;
	if (!_cmd_resp_check(dev, "AT+CPIN?\r\n", "+CPIN: READY", 10, 200, 500)) return SIM800_SIM_ERROR;
	if (!_cmd_resp_check(dev, "AT+CREG?\r\n", "+CREG: 0,1", 20, 200, 500)) return SIM800_REG_ERROR;

	return SIM800_READY;
}

int sim800_attach(sim800_t *dev)
{
	if (!_cmd_resp_check(dev, "AT+CGATT=1\r\n", "OK", 20, 5000, 1000)) return SIM800_ATT_ERROR;

	return SIM800_READY;
}

int sim800_detach(sim800_t *dev)
{
	if (!_cmd_resp_check(dev, "AT+CGATT=0\r\n", "OK", 2, 10000, 2000)) return SIM800_ATT_ERROR;

	return SIM800_READY;
}

// todo: this routine doesn't work well!
// often returns error even though the modem response is alright!
int sim800_rssi(sim800_t *dev)
{
	char rssi_str[20];
	unsigned int rssi = 0;
	unsigned int ber = 0;

	if (!_cmd_read(dev, "AT+CSQ\r\n", rssi_str, sizeof(rssi_str), 3, 200)) return SIM800_UNRESPONSIVE;
	if (sscanf(rssi_str, "+CSQ: %u,%u", &rssi, &ber) != 2) return SIM800_RSSI_ERROR;
	if (rssi == 99) return SIM800_RSSI_ERROR;

	return (int)rssi;
}

int sim800_set_apn(sim800_t *dev, const char *apn, const char *user, const char *password)
{
	char apn_str[100];
	sprintf(apn_str, "AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n", apn, user, password);
	//sprintf(apn_str, "AT+CSTT=\"%s\"\r\n", apn);

	// if APN is already set this will return an error but it's not a real error
	if (!_cmd_resp_check(dev, apn_str, "OK", 5, 200, 1000))
		return SIM800_APN_ERROR;
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

	//if (sim800_set_apn(dev, "payandgo.o2.co.uk", "payandgo", "password") != SIM800_OK) return SIM800_APN_ERROR;
	if (sim800_set_apn(dev, "pp.vodafone.co.uk", "web", "web") != SIM800_OK) return SIM800_APN_ERROR;
	//if (sim800_set_apn(dev, "everywhere", "", "") != SIM800_OK) return SIM800_APN_ERROR;

	return SIM800_READY;
}

int sim800_operator_name(sim800_t *dev, char *name)
{
	char operator_str[40];
	if (!_cmd_resp_check(dev, "AT+COPS=3,0\r\n", "OK", 4, 1000, 500)) return SIM800_FORMAT_ERROR;
	if (!_cmd_read(dev, "AT+COPS?\r\n", operator_str, sizeof(operator_str), 3, 1000)) return SIM800_NO_OPERATOR;

	// find opening quote
	char *open_quote = strchr(operator_str, '"');
	if (open_quote == NULL) return SIM800_OPERATOR_ERROR;
	// find closing quote
	char *close_quote = strchr(open_quote+1, '"');
	if (close_quote == NULL) return SIM800_OPERATOR_ERROR;
	// extract the string and copy to name
	int name_len = close_quote - open_quote - 1;
	memcpy(name, (open_quote + 1), name_len);
	// capitalize it
	if (*name > 96 && *name < 123) *name -= 32;
	// terminate it
	*(name + name_len) = 0;

	return SIM800_READY;
}

int sim800_gprs_connect(sim800_t *dev)
{
	char ip_addr_str[24];
	unsigned int ip0 = 0, ip1 = 0, ip2 = 0, ip3 = 0;

	int signal_strength = sim800_rssi(dev);
	xtimer_usleep(5 * 1000U); // wait a bit after sim800_rssi()
	//if (signal_strength < 5 || signal_strength == SIM800_RSSI_ERROR) return SIM800_POOR_SIGNAL;

	if (!_cmd_resp_check(dev, "AT+CIICR\r\n", "OK", 2, 20000, 10000)) return SIM800_GPRS_ERROR;
	if (!_cmd_read(dev, "AT+CIFSR\r\n", ip_addr_str, sizeof(ip_addr_str), 3, 300)) return SIM800_NO_IP;
	
	if (sscanf(ip_addr_str, "%u.%u.%u.%u", &ip0, &ip1, &ip2, &ip3) != 4) return SIM800_IP_ERROR;

	return SIM800_GPRS_READY;
}

int sim800_gprs_disconnect(sim800_t *dev)
{
	if (!_cmd_resp_check(dev, "AT+CIPSHUT\r\n", "SHUT OK", 3, 10000, 2000)) return SIM800_UNRESPONSIVE;

	return SIM800_READY;
}

int sim800_powerdown(sim800_t *dev)
{
	sim800_gprs_disconnect(dev);
	sim800_detach(dev);
	if (!_cmd_resp_check(dev, "AT+CPOWD=1\r\n", "POWER DOWN", 2, 10000, 2000)) return SIM800_UNRESPONSIVE;

	return SIM800_READY;
}

// in RAM modes, buffers must be null terminated
// in FILE modes, the file size is determined using fstat
int sim800_http(sim800_t *dev,
	const char* host, uint16_t port,
	const char *req_head, void *req_body,
	char *res_head, void *res_body,
	int req_mode, int res_mode,
	int head_maxsize, int body_maxsize)
{
	int status;
	char cipstart_str[100];
	uint8_t terminate = 0x1a;

	if (!(req_mode == SIM800_READ_FROM_RAM || req_mode == SIM800_READ_FROM_FILE)) return SIM800_INVALID_MODE;
	if (!(res_mode == SIM800_WRITE_TO_RAM || res_mode == SIM800_WRITE_TO_FILE)) return SIM800_INVALID_MODE;	

	sprintf(cipstart_str, "AT+CIPSTART=\"TCP\",\"%s\",\"%u\"\r\n", host, port);
	if (!_cmd_resp_check(dev, cipstart_str, "OK", 3, 500, 1000)) { status = SIM800_TCP_NOCIPSTART; goto end; }

	// now wait for connection to be established!
	if (_wait_for_sequence(dev, "CONNECT OK\r\n", 10000) < 0) { status = SIM800_TCP_NOCONNECT; goto end; }

	// now we can send data!
	// initiate send and wait for the > prompt
	uart_write(dev->uart, (const uint8_t*)"AT+CIPSEND\r\n", strlen("AT+CIPSEND\r\n"));
	if (_wait_for_sequence(dev, "> ", 1000) < 0) { status = SIM800_NO_PROMPT; goto end; }
	
	// write request head
	uart_write(dev->uart, (uint8_t*)req_head, strlen(req_head));

	// write request body (if given)
	if (req_body != NULL) {
		if (req_mode == SIM800_READ_FROM_RAM) {
			uart_write(dev->uart, (uint8_t*)req_body, strlen((char*)req_body));
		}
		else if (req_mode == SIM800_READ_FROM_FILE) {
			//char tempbuf[64];
			int fd = *((int*)req_body);
			int res;
			char c;
			/*
			do { // write in 64 byte chunks until file ends
				res = vfs_read(fd, tempbuf, sizeof(tempbuf));
				if (res < 0) { status = SIM800_FILE_READ_ERROR; goto end;}
				uart_write(dev->uart, tempbuf, res);
			} while (res == sizeof(tempbuf));
			*/
			while (res = vfs_read(fd, &c, 1)) uart_write(dev->uart, c, 1);
		}
	}

	// write termination character Ctrl-Z
	uart_write(dev->uart, &terminate, sizeof(terminate));

	// now wait for confirmation!
	if (_wait_for_sequence(dev, "SEND OK\r\n", 10000) < 0) { status = SIM800_TCP_NOSEND; goto end; }

	// read headers
	status = _receive_tcp(dev, res_head, head_maxsize, "\r\n\r\n", 20000);
	if (status < 0) goto end;

	// skip reading body if response pointer is null or size 0
	if (res_body == NULL || body_maxsize <= 0) { status = SIM800_OK; goto end; }

	// read body
	if (res_mode == SIM800_WRITE_TO_RAM) {
		char *response = (char*) res_body;
		status = _receive_tcp(dev, response, body_maxsize, "\r\nCLOSED\r\n", 20000);
	}
	else if (res_mode == SIM800_WRITE_TO_FILE) {
		int fd = *((int*)res_body);
		status = _receive_tcp_file(dev, fd, body_maxsize, "\r\nCLOSED\r\n", 20000);
	}
	if (status < 0) goto end;
	status = SIM800_OK;

end:
	_cmd_resp_check(dev, "AT+CIPCLOSE\r\n", "CLOSE OK", 3, 2000, 1000);
	return status;
}