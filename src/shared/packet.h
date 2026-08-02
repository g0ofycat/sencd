// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/shared/packet.h: Packet protocol for everything
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#ifndef PACKET_H
#define PACKET_H

#include <arpa/inet.h>

#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../debug/logs.h"

//--============
// -- CONSTS
//--============

#define CURRENT_PROTOCOL_VERSION 1

#define MAX_PACKET_SIZE 0x1000000

//--============
// -- TYPEDEFS
//--============

typedef enum {
	PACKET_CLIENT_HELLO,
	PACKET_SERVER_HELLO,

	PACKET_AUTH_REQUEST,
	PACKET_AUTH_RESPONSE,
	PACKET_AUTH_SUCCESS,
	PACKET_AUTH_FAIL,

	PACKET_KEY_EXCHANGE,
	PACKET_KEY_CONFIRM,

	PACKET_PING,
	PACKET_PONG,

	PACKET_TUNNEL_DATA,

	PACKET_DISCONNECT,

	PACKET_ERROR
} PACKET_HEADER_T;

typedef struct {
	uint32_t payload_length;
	uint8_t version;
	uint8_t type;
} PACKET_HEADER;

typedef struct {
	uint8_t *payload;
	PACKET_HEADER header;
} PACKET;

typedef struct {
	uint8_t *payload;
	uint32_t payload_length;
	PACKET_HEADER_T header_type;
	uint8_t header_version;
} PACKET_CONSTRUCTOR_T;

//--============
// -- DEFINITONS
//--============

/// @brief init a packet
/// @param *packet
void packet_init(PACKET *packet);

/// @brief destroy all packet data
/// @param *packet
void packet_destroy(PACKET *packet);

/// @brief send a packet to the given socket
/// @param socket
/// @param *packet
/// @return int: success bool
int packet_send(int socket, PACKET *packet);

/// @brief recv a packet from the given socket
/// @param socket
/// @param *packet
/// @return int: success bool
int packet_receive(int socket, PACKET *packet);

/// @brief wrapper for initializing and formatting a packet
/// @return int: success bool
PACKET packet_construct(PACKET_CONSTRUCTOR_T data);

#endif
