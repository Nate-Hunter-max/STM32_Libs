/**
 * @file CryptoSchizo.c
 * @brief Implementation of data visualization and encoding utilities
 * @details Contains Base64 encoding/decoding and RandomArt generation
 */

#include "CryptoSchizo.h"
#include <string.h>

/* Constants */
static const char base64_table[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static const char art_chars[] = " .o+=*BOX@%&#/^SE";

#define ART_WIDTH  17  /* Fixed canvas width (OpenSSH standard) */
#define ART_HEIGHT 9   /* Fixed canvas height (OpenSSH standard) */

/// SHA-256 constants (first 32 bits of the fractional parts of the cube roots of the first 64 primes)
static const uint32_t k[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};


/// Initial hash values (first 32 bits of the fractional parts of the square roots of the first 8 primes)
static const uint32_t initial_hash[8] = {
    0x6a09e667, 0xbb67ae85,
    0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c,
    0x1f83d9ab, 0x5be0cd19
};

/**
 * @brief Generate OpenSSH-style RandomArt
 * 
 * @param data Input binary data
 * @param len Data length in bytes
 * @param output Output buffer (min size: (ART_HEIGHT*(ART_WIDTH+3)+3))
 */
void CryptoSchizo_GenerateArt(const uint8_t* data, uint16_t len, char* output)
{
    uint8_t grid[ART_HEIGHT][ART_WIDTH] = {0};

    int x = ART_WIDTH / 2;
    int y = ART_HEIGHT / 2;

    // Start and end markers
    grid[y][x] = 15; // 'S'

    for (uint16_t i = 0; i < len; ++i) {
        uint8_t byte = data[i];

        // Each byte generates 4 steps (2 bits per step)
        for (int j = 0; j < 4; ++j) {
            uint8_t dir = (byte >> (j * 2)) & 0x03;
            switch (dir) {
                case 0: if (y > 0) --y; break;       // Up
                case 1: if (x < ART_WIDTH - 1) ++x; break; // Right
                case 2: if (y < ART_HEIGHT - 1) ++y; break; // Down
                case 3: if (x > 0) --x; break;       // Left
            }

            if (grid[y][x] < 14)
                ++grid[y][x];
        }
    }

    // Mark end position
    grid[y][x] = 16; // 'E'

    // Render output
    char* ptr = output;
    *ptr++ = '+'; for (int i = 0; i < ART_WIDTH; ++i) *ptr++ = '-'; *ptr++ = '+'; *ptr++ = '\n';

    for (int y = 0; y < ART_HEIGHT; ++y) {
        *ptr++ = '|';
        for (int x = 0; x < ART_WIDTH; ++x) {
            *ptr++ = art_chars[grid[y][x]];
        }
        *ptr++ = '|';
        *ptr++ = '\n';
    }

    *ptr++ = '+'; for (int i = 0; i < ART_WIDTH; ++i) *ptr++ = '-'; *ptr++ = '+'; *ptr++ = '\n';
    *ptr = '\0';
}

/**
 * @brief Encode binary data to Base64
 * @param data Input binary data
 * @param length Data length in bytes
 * @param output Output buffer (min size: 4 * ((length + 2) / 3) + 1)
 * @return Pointer to output buffer
 * @note Implements RFC 4648 Base64 encoding
 */
char* CryptoSchizo_Base64Encode(const uint8_t* data, uint16_t length, char* output) 
{
    uint16_t i = 0, j = 0;
    
    while (i < length) {
        uint32_t octet_a = i < length ? data[i++] : 0;
        uint32_t octet_b = i < length ? data[i++] : 0;
        uint32_t octet_c = i < length ? data[i++] : 0;
        
        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;
        
        output[j++] = base64_table[(triple >> 18) & 0x3F];
        output[j++] = base64_table[(triple >> 12) & 0x3F];
        output[j++] = i > length + 1 ? '=' : base64_table[(triple >> 6) & 0x3F];
        output[j++] = i > length ? '=' : base64_table[triple & 0x3F];
    }
    
    output[j] = '\0';
    return output;
}

/**
 * @brief Decode Base64 string to binary
 * @param input Base64 string (null-terminated)
 * @param output Binary output buffer
 * @return Decoded data length in bytes, 0 on error
 * @note Supports padding characters ('=')
 */
uint16_t CryptoSchizo_Base64Decode(const char* input, uint8_t* output) 
{
    uint16_t i = 0, j = 0;
    uint8_t sextet[4];
    
    while (input[i] != '\0') {
        if (input[i] == '=') break; /* Padding reached */
        
        const char* ptr = strchr(base64_table, input[i]);
        if (!ptr) return 0; /* Invalid character */
        sextet[i % 4] = ptr - base64_table;
        i++;
        
        /* Process every 4 characters */
        if (i % 4 == 0) {
            output[j++] = (sextet[0] << 2) | (sextet[1] >> 4);
            output[j++] = (sextet[1] << 4) | (sextet[2] >> 2);
            output[j++] = (sextet[2] << 6) | sextet[3];
        }
    }
    
    /* Handle remaining bytes with padding */
    if (i % 4 == 2) {
        output[j++] = (sextet[0] << 2) | (sextet[1] >> 4);
    } 
    else if (i % 4 == 3) {
        output[j++] = (sextet[0] << 2) | (sextet[1] >> 4);
        output[j++] = (sextet[1] << 4) | (sextet[2] >> 2);
    }
    
    return j;
}

/// Rotate right: manual implementation
static inline uint32_t rotr(uint32_t value, uint8_t bits) {
    return (value >> bits) | (value << (32 - bits));
}

/**
 * @brief Compute SHA-256 hash of input data
 *
 * @param input Pointer to input data
 * @param len Length of input data in bytes
 * @param output Pointer to 32-byte buffer for the resulting hash
 * @return Pointer to the output buffer
 *
 * @note Implements FIPS PUB 180-4 SHA-256 hashing
 */
uint8_t* CryptoSchizo_SHA256(const uint8_t* input, uint32_t len, uint8_t* output)
{
    uint32_t h[8];
    memcpy(h, initial_hash, sizeof(h));

    uint64_t bitlen = (uint64_t)len * 8;
    uint32_t total_len = ((len + 9 + 63) / 64) * 64;
    uint8_t block[64];

    for (uint32_t offset = 0; offset < total_len; offset += 64) {
        memset(block, 0, 64);

        if (offset + 64 <= len) {
            memcpy(block, input + offset, 64);
        } else {
            uint32_t remain = len - offset;
            if (remain > 0) memcpy(block, input + offset, remain);
            block[remain] = 0x80;
            if (offset + 64 >= total_len - 64) {
                uint64_t bitlen_be =
                    ((bitlen >> 56) & 0xFF) |
                    ((bitlen >> 40) & 0xFF00) |
                    ((bitlen >> 24) & 0xFF0000) |
                    ((bitlen >> 8) & 0xFF000000) |
                    ((bitlen & 0xFF) << 56) |
                    ((bitlen & 0xFF00) << 40) |
                    ((bitlen & 0xFF0000) << 24) |
                    ((bitlen & 0xFF000000) << 8);
                memcpy(block + 56, &bitlen_be, 8);
            }
        }

        uint32_t w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = (block[i * 4] << 24) | (block[i * 4 + 1] << 16) | (block[i * 4 + 2] << 8) | block[i * 4 + 3];
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = rotr(w[i - 15], 7) ^ rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
            uint32_t s1 = rotr(w[i - 2], 17) ^ rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16] + s0 + w[i - 7] + s1;
        }

        uint32_t a = h[0], b = h[1], c = h[2], d = h[3];
        uint32_t e = h[4], f = h[5], g = h[6], hh = h[7];

        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = hh + S1 + ch + k[i] + w[i];
            uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = S0 + maj;

            hh = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        h[0] += a; h[1] += b; h[2] += c; h[3] += d;
        h[4] += e; h[5] += f; h[6] += g; h[7] += hh;
    }

    for (int i = 0; i < 8; ++i) {
        output[i * 4 + 0] = (h[i] >> 24) & 0xFF;
        output[i * 4 + 1] = (h[i] >> 16) & 0xFF;
        output[i * 4 + 2] = (h[i] >> 8) & 0xFF;
        output[i * 4 + 3] = h[i] & 0xFF;
    }

    return output;
}
