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
	uint64_t clients_connected;
	uint16_t port;

	int socket;
	int running;
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
int server_shutdown(SERVER_T* server);

#endif
