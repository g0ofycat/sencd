// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/server/server_core.h: Main functions for initializing the server
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#ifndef CORE_SERVER_H
#define CORE_SERVER_H

#include <arpa/inet.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "../debug/logs.h"

//--============
// -- CONSTS
//--============

#define SERVER_DEFAULT_PORT 8080

//--============
// -- TYPEDEFS
//--============

typedef struct {
	int socket;
	int running;
	uint16_t port;
} SERVER_T;

//--============
// -- DEFINITIONS
//--============

/// @brief init server data before starting
/// @param *server
void server_init(SERVER_T *server);

/// @brief start the server and listen to client connections
/// @param *server
/// @return int: success bool
int server_start(SERVER_T *server, uint16_t port);

/// @brief accept all clients to the server
/// @param *server
/// @return int: success bool
int server_accept(SERVER_T *server);

/// @brief shutdown server and disconnect all clients
/// @param *server
/// @return int: success bool
int server_shutdown(SERVER_T *server);

#endif
