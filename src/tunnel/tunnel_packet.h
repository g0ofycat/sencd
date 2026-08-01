// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/shared/tunnel_packet.h: Tunnel a packet through the session ID
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#ifndef TUNNEL_PACKET_H
#define TUNNEL_PACKET_H

#include <endian.h>
#include <stddef.h>
#include <stdint.h>

#include "../debug/logs.h"
#include "../shared/security/crypto.h"

//--============
// -- CONSTS
//--============

#define TUNNEL_HEADER_SIZE                                                     \
	(sizeof(uint64_t) + sizeof(uint64_t)) // session_id + counter

#define TUNNEL_PACKET_MAX_SIZE 65536
#define TUNNEL_PACKET_MAX_PLAINTEXT                                            \
	(TUNNEL_PACKET_MAX_SIZE - TUNNEL_HEADER_SIZE - TUNNEL_AEAD_TAG_SIZE)

//--============
// -- TYPEDEFS
//--============

typedef struct {
	uint64_t session_id;
	uint64_t counter;
	const unsigned char *plaintext;
	size_t plaintext_len;
} TUNNEL_PACKET_ENCODE_T;

//--============
// -- DEFINITIONS
//--============

/// @brief encode a tunnel data payload: [session_id][counter][ciphertext+tag]
/// @param *in: tunnel packet to encode
/// @param *ctx: crypto context holding tx_key
/// @param *out_buffer: output buffer, must be at least plaintext_len +
/// TUNNEL_HEADER_SIZE + TUNNEL_AEAD_TAG_SIZE
/// @param *out_length: total bytes written to out_buffer
/// @return int: success bool
int tunnel_packet_encode(const TUNNEL_PACKET_ENCODE_T *in,
						 CRYPTO_CONTEXT_T *ctx, unsigned char *out_buffer,
						 size_t *out_length);

/// @brief read the session_id out of a raw tunnel payload without decrypting
/// @param *buffer, buffer_len: buffer to read
/// @param *session_id_out: session id to read from
/// @return int: success bool
int tunnel_packet_peek_session_id(const unsigned char *buffer,
								  size_t buffer_len, uint64_t *session_id_out);

/// @brief decode + decrypt a tunnel data payload
/// @param *buffer, buffer_len: buffer to decode
/// @param *ctx: crypto context holding rx_key
/// @param *out_plaintext: output buffer, must be at least buffer_len -
/// TUNNEL_HEADER_SIZE - TUNNEL_AEAD_TAG_SIZE
/// @param *out_length: bytes written to out_plaintext
/// @return int: success bool
int tunnel_packet_decode(const unsigned char *buffer, size_t buffer_len,
						 CRYPTO_CONTEXT_T *ctx, unsigned char *out_plaintext,
						 unsigned long long *out_length);

#endif
