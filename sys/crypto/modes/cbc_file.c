/*
 * Copyright (C) 2015 Freie Universität Berlin
 *               2017 Georgios Psimenos
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */


#include <string.h>
#include "crypto/modes/cbc_file.h"
#include "vfs.h"

static int hex2int(char c)
{
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + c - 'a';
    if (c >= 'A' && c <= 'F') return 10 + c - 'A';
    return -1;
}

static char nibble2hex(uint8_t n)
{
    if (n >= 0 && n <= 9) return '0' + n;
    if (n >= 10 && n <= 15) return 'a' + n - 10;
    return 0;
}

static void binary2hex(uint8_t *data, char *hexstring, int size)
{
    char c;
    while (size--) {
        *hexstring++ = nibble2hex((*data >> 4) & 0xF);
        *hexstring++ = nibble2hex((*data++) & 0xF);
    }
}

static int hex2binary(uint8_t *data, char *hexstring, int size)
{
    int val;
    while (size--) {
        val = hex2int(*hexstring++);
        if (val < 0) return -1;
        *data = 16 * val;
        val = hex2int(*hexstring++);
        if (val < 0) return -1;
        *(data++) += val;
    }
    return 0;
}

int cipher_encrypt_cbc_file(cipher_t* cipher, uint8_t iv[16],
                       int fd_in, size_t length, int fd_out, int mode)
{
    int res;
    size_t offset = 0;
    uint8_t block_size, input_block[CIPHER_MAX_BLOCK_SIZE] = {0},
            output_block[CIPHER_MAX_BLOCK_SIZE] = {0}, *output_block_last;
    char hex_block[2*CIPHER_MAX_BLOCK_SIZE];

    block_size = cipher_get_block_size(cipher);
    if (length % block_size != 0) {
        return CIPHER_ERR_INVALID_LENGTH;
    }

    output_block_last = iv;
    do {
        // read input_block from file, file cursor advances automatically
        res = vfs_read(fd_in, input_block, block_size);
        if (res < block_size) return CIPHER_ERR_FILE_READ_ERROR;

        /* CBC-Mode: XOR plaintext with ciphertext of (n-1)-th block */
        for (int i = 0; i < block_size; ++i) {
            input_block[i] ^= output_block_last[i];
        }

        if (cipher_encrypt(cipher, input_block, output_block) != 1) {
            return CIPHER_ERR_ENC_FAILED;
        }

        // write output_block to file, file cursor advances automatically
        if (mode == CIPHER_MODE_BINARY) {
            res = vfs_write(fd_out, output_block, block_size);
            if (res < block_size) return CIPHER_ERR_FILE_WRITE_ERROR;
        }
        else if (mode == CIPHER_MODE_HEX) {
            binary2hex(output_block, hex_block, block_size);
            res = vfs_write(fd_out, hex_block, 2*block_size);
            if (res < block_size) return CIPHER_ERR_FILE_WRITE_ERROR;
        }

        output_block_last = output_block;
        offset += block_size;
    } while (offset < length);

    return offset;
}


int cipher_decrypt_cbc_file(cipher_t* cipher, uint8_t iv[16],
                       int fd_in, size_t length, int fd_out, int mode)
{
    int res;
    size_t offset = 0;
    uint8_t block_size, input_block[CIPHER_MAX_BLOCK_SIZE] = {0},
            output_block[CIPHER_MAX_BLOCK_SIZE] = {0},
            input_block_last[CIPHER_MAX_BLOCK_SIZE] = {0};
    char hex_block[2*CIPHER_MAX_BLOCK_SIZE];


    block_size = cipher_get_block_size(cipher);
    if (length % block_size != 0) {
        return CIPHER_ERR_INVALID_LENGTH;
    }

    memcpy(input_block_last, iv, block_size);
    do {
        // read input_block from file, file cursor advances automatically
        if (mode == CIPHER_MODE_BINARY) {
            res = vfs_read(fd_in, input_block, block_size);
            if (res < block_size) return CIPHER_ERR_FILE_READ_ERROR;
        }
        else if (mode == CIPHER_MODE_HEX) {
            res = vfs_read(fd_in, hex_block, 2*block_size);
            if (res < 2*block_size) return CIPHER_ERR_FILE_READ_ERROR;
            hex2binary(input_block, hex_block, block_size);
        }
        else return CIPHER_ERR_INVALID_MODE;
        

        if (cipher_decrypt(cipher, input_block, output_block) != 1) {
            return CIPHER_ERR_DEC_FAILED;
        }

        /* CBC-Mode: XOR plaintext with ciphertext of (n-1)-th block */
        for (uint8_t i = 0; i < block_size; ++i) {
            output_block[i] ^= input_block_last[i];
        }

        // write output_block to file, file cursor advances automatically
        res = vfs_write(fd_out, output_block, block_size);
        if (res < block_size) return CIPHER_ERR_FILE_WRITE_ERROR;

        memcpy(input_block_last, input_block, block_size);
        offset += block_size;
    } while (offset < length);

    return offset;
}
