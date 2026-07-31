// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/tunnel/udp_tunnel.h: Tunnel for sending client network packets to server
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#ifndef UDP_TUNNEL_H
#define UDP_TUNNEL_H

#include <arpa/inet.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "../debug/logs.h"

//--============
// -- TYPEDEFS
//--============

typedef struct {
	uint64_t session_id;
	int socket;
	uint8_t peer_known;
	struct sockaddr_in peer_addr;
} UDP_TUNNEL_T;

//--============
// -- DEFINITIONS
//--============

/// @brief initialize UDP tunnel for the current session ID
/// @param *tunnel
/// @param session_id
void udp_tunnel_init(UDP_TUNNEL_T *tunnel, uint64_t session_id);

/// @brief open client tunnel
/// @param *tunnel
/// @param *server_ip
/// @param port
/// @return int: success bool
int udp_tunnel_client_open(UDP_TUNNEL_T *tunnel, const char *server_ip,
						   uint16_t port);

/// @brief close tunnel
/// @param *tunnel
void udp_tunnel_close(UDP_TUNNEL_T *tunnel);

#endif
