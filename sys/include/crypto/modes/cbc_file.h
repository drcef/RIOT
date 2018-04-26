/*
 * Copyright (C) 2015 Freie Universität Berlin
 *               2017 Georgios Psimenos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#ifndef CRYPTO_MODES_CBC_FILE_H
#define CRYPTO_MODES_CBC_FILE_H

#include "crypto/ciphers.h"

#define CIPHER_ERR_FILE_READ_ERROR    -11
#define CIPHER_ERR_FILE_WRITE_ERROR   -12
#define CIPHER_ERR_INVALID_MODE       -13

#define CIPHER_MODE_BINARY	1
#define CIPHER_MODE_HEX		2

#ifdef __cplusplus
extern "C" {
#endif


int cipher_encrypt_cbc_file(cipher_t* cipher, uint8_t iv[16], int fd_in,
                       size_t length, int fd_out, int mode);


int cipher_decrypt_cbc_file(cipher_t* cipher, uint8_t iv[16], int fd_in,
                       size_t length, int fd_out, int mode);

#ifdef __cplusplus
}
#endif

#endif /* CRYPTO_MODES_CBC_H */
