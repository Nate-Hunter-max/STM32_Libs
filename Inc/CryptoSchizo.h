/**
 * @file CryptoSchizo.h
 * @brief Data visualization and encoding utilities for embedded systems
 *
 * @author [Crypto Schizo]
 * @date [30.04.2025]
 * @version 1.0
 */

#ifndef CRYPTO_SCHIZO_H
#define CRYPTO_SCHIZO_H

#include <stdint.h>

/**
 * @brief Generate OpenSSH-style RandomArt
 * @param data Input binary data
 * @param len Data length in bytes
 * @param output Output buffer (min size: (height*(width+2)+3)
 */
void CryptoSchizo_GenerateArt(const uint8_t* data, uint16_t len, char* output);

/**
 * @brief Encode binary data to Base64
 * @param data Input binary data
 * @param length Data length in bytes
 * @param output Output buffer (min size: 4 * ((length + 2) / 3) + 1)
 * @return Pointer to output buffer
 */
char* CryptoSchizo_Base64Encode(const uint8_t* data, uint16_t length, char* output);

/**
 * @brief Decode Base64 string to binary
 * @param input Base64 string (null-terminated)
 * @param output Binary output buffer
 * @return Decoded data length in bytes, 0 on error
 */
uint16_t CryptoSchizo_Base64Decode(const char* input, uint8_t* output);

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
uint8_t* CryptoSchizo_SHA256(const uint8_t* input, uint32_t len, uint8_t* output);

#endif // CRYPTO_SCHIZO_H
