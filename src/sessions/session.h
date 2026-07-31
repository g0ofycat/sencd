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

#include "../tunnel/udp_tunnel.h"

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
	uint64_t session_id;
	UDP_TUNNEL_T udp;

	time_t created;
	time_t last_seen;

	uint8_t auth_nonce[AUTH_NONCE_SIZE];
	uint8_t session_nonce[SESSION_NONCE_SIZE];
	unsigned char trusted_peer_key[CRYPTO_PUBLIC_KEY_SIZE];
	uint8_t has_trusted_peer_key;
	uint8_t protocol_version;

	SESSION_STATE_T state;
	SESSION_ROLE_T role;
	int socket;

	char ip[INET_ADDRSTRLEN];
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
/// @param *identity
/// @param *trusted_peer_key
/// @return int: success bool
int session_create(SESSION_T *session, SESSION_ROLE_T role, int socket,
				   const char *ip, const CRYPTO_CONTEXT_T *identity,
				   const unsigned char *trusted_peer_key);

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
