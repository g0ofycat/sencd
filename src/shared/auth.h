// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/shared/auth.h: Authentication typedefs for sessions
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#ifndef AUTH_H
#define AUTH_H

#include <stdint.h>

#include "security/crypto.h"

//--============
// -- TYPEDEFS
//--============

typedef struct {
	uint8_t public_key[CRYPTO_PUBLIC_KEY_SIZE];
	uint8_t nonce[32];
} AUTH_CHALLENGE_T;

typedef struct {
	uint8_t public_key[CRYPTO_PUBLIC_KEY_SIZE];
	uint8_t signature[CRYPTO_SIGNATURE_SIZE];
} AUTH_RESPONSE_T;

#endif
