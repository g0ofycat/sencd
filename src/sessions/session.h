// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/sessions/session.h: Logic for client->server connection pipeline
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#ifndef SESSION_H
#define SESSION_H

#include <arpa/inet.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "../client/client_core.h"
#include "../shared/packet.h"

#include "../shared/auth.h"
#include "../shared/security/crypto.h"

//--============
// -- CONSTS
//--============

#define KEY_SIZE 32

#define TRUSTED_SERVER_KEY_SIZE CRYPTO_PUBLIC_KEY_SIZE

static const uint8_t TRUSTED_SERVER_KEY[TRUSTED_SERVER_KEY_SIZE] = {
	0x8A, 0x3F, 0x91, 0xC4, 0x27, 0x6D, 0xE8, 0x52,
	0xB1, 0x0F, 0x74, 0xA9, 0x33, 0xD6, 0x48, 0xFE,
	0x19, 0x85, 0xCB, 0x60, 0x2E, 0x97, 0x4A, 0xD1,
	0x5C, 0xE3, 0x06, 0xB8, 0x71, 0x4F, 0xAA, 0x23
};	// TODO: obv make this client side verif of server

//--============
// -- TYPEDEFS
//--============

typedef enum {
	SESSION_CONNECTING,
	SESSION_WAIT_AUTH,
	SESSION_AUTHENTICATED,
	SESSION_ESTABLISHED,
	SESSION_DISCONNECTED
} SESSION_STATE_T;

typedef enum { SESSION_SERVER, SESSION_CLIENT } SESSION_ROLE_T;

typedef struct {
	CRYPTO_CONTEXT_T crypto;
	uint8_t auth_nonce[AUTH_NONCE_SIZE];

	char ip[INET_ADDRSTRLEN];
	uint64_t session_id;

	time_t created;
	time_t last_seen;

	uint8_t protocol_version;

	SESSION_STATE_T state;
	SESSION_ROLE_T role;

	int socket;
} SESSION_T;

//--============
// -- DEFINITIONS
//--============

/// @brief initialize a session
/// @param *session
/// @param role
void session_init(SESSION_T *session, SESSION_ROLE_T role);

/// @brief create a new session
/// @param *session
/// @param role
/// @param socket
/// @param *ip
/// @return int: success bool
int session_create(SESSION_T *session, SESSION_ROLE_T role, int socket,
		const char *ip);

/// @brief destroy a session
/// @param *session
void session_destroy(SESSION_T *session);

/// @brief auth the server (sends server auth challenge)
/// @param *session
/// @return int: success bool
int session_authenticate_server(SESSION_T *session);

/// @brief handle the initial connection handshake [server]
/// @param *session
/// @return int: success bool
int session_server_connect(SESSION_T *session);

/// @brief auth the client
/// @param *session
/// @return int: success bool
int session_authenticate_client(SESSION_T *session);

/// @brief handle the initial connection handshake [client]
/// @param *session
/// @return int: success bool
int session_client_connect(SESSION_T *session);

/// @brief verify session signature
/// @param *session
/// @return int: success bool
int session_verify_client(SESSION_T *session);

#endif
