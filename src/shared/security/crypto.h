// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/shared/security/crypto.h: Cryptography for connecting to a session
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#ifndef CRYPTO_H
#define CRYPTO_H

#include <sodium.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//--============
// -- CONSTS
//--============

#define CRYPTO_PUBLIC_KEY_SIZE 32
#define CRYPTO_PRIVATE_KEY_SIZE 64
#define CRYPTO_SESSION_KEY_SIZE 32
#define CRYPTO_SIGNATURE_SIZE 64

//--============
// -- TYPEDEFS
//--============

typedef struct {
	unsigned char public_key[CRYPTO_PUBLIC_KEY_SIZE];
	unsigned char private_key[CRYPTO_PRIVATE_KEY_SIZE];

	unsigned char peer_public_key[CRYPTO_PUBLIC_KEY_SIZE];

	unsigned char tx_key[CRYPTO_SESSION_KEY_SIZE];
	unsigned char rx_key[CRYPTO_SESSION_KEY_SIZE];

	unsigned char eph_public_key[crypto_kx_PUBLICKEYBYTES];
	unsigned char eph_private_key[crypto_kx_SECRETKEYBYTES];

	uint64_t tx_nonce;
	uint64_t rx_nonce;

	uint8_t verified;
} CRYPTO_CONTEXT_T;

//--============
// -- DEFINITIONS
//--============

/// @brief initialize the libsodium cryptography library
void crypto_init(void);

/// @brief initialize a crypto context
/// @param *ctx: crypto context to initialize
void crypto_context_init(CRYPTO_CONTEXT_T *ctx);

/// @brief generate a cryptographically secure random nonce
/// @param *nonce: output nonce buffer
/// @param size: nonce size in bytes
void crypto_generate_nonce(uint8_t *nonce, size_t size);

/// @brief generate a long-term public/private identity key pair
/// @param *ctx: crypto context to populate
/// @return int: success status
int crypto_generate_identity(CRYPTO_CONTEXT_T *ctx);

/// @brief sign a message using the context's private key
/// @param *ctx: crypto context containing the private key
/// @param *message: message to sign
/// @param message_length: length of the message in bytes
/// @param signature: output buffer (CRYPTO_SIGNATURE_SIZE bytes)
/// @return int: success status
int crypto_sign_message(CRYPTO_CONTEXT_T *ctx, const unsigned char *message,
						size_t message_length,
						unsigned char signature[CRYPTO_SIGNATURE_SIZE]);

/// @brief verify a message signature using a public key
/// @param public_key: public key used for verification
/// @param *message: original message
/// @param message_length: length of the message in bytes
/// @param signature: signature to verify (CRYPTO_SIGNATURE_SIZE bytes)
/// @return int: success status
int crypto_verify_message(
	const unsigned char public_key[CRYPTO_PUBLIC_KEY_SIZE],
	const unsigned char *message, size_t message_length,
	const unsigned char signature[CRYPTO_SIGNATURE_SIZE]);

#endif
