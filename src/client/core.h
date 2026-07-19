#ifndef START_H
#define START_H

#include <arpa/inet.h>

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "../debug/logs.h"

//--============
// -- CONSTS
//--============

#define DEFAULT_PORT 8080

//--============
// -- TYPEDEFS
//--============

typedef enum {
	CONNECTION_CONNECTING,
	CONNECTION_HANDSHAKING,
	CONNECTION_AUTHENTICATING,
	CONNECTION_CONNECTED,

	CONNECTION_DISCONNECTING,
	CONNECTION_DISCONNECTED
} CONNECTION_STATE;

typedef struct {
	char ip[64];

	uint64_t packets_sent;
	uint64_t packets_received;

	uint64_t bytes_sent;
	uint64_t bytes_received;

	uint32_t session_id;
	uint16_t port;

	int socket;
	CONNECTION_STATE state;
} CONNECTION_T;

//--============
// -- DEFINITIONS
//--============

/// @brief init current socket and state
/// @param *connection
void connection_init(CONNECTION_T *connection);

/// @brief connect to a specific ip addr
/// @param *connection
/// @param *ip
/// @param port
/// @return int: success bool
int connection_connect(CONNECTION_T *connection, const char *ip, uint16_t port);

/// @brief disconnect from the current server
/// @param *connection
/// @return int: success bool
int connection_disconnect(CONNECTION_T *connection);

/// @brief send data to a connection
/// @param *connection
/// @param *buffer
/// @param length
/// @return ssize_t: send() output
ssize_t connection_send(CONNECTION_T *connection, const void *buffer, size_t length);

/// @brief receive data from a connection
/// @param *connection
/// @param *buffer
/// @param length
/// @return ssize_t: recv() output
ssize_t connection_receive(CONNECTION_T *connection, void *buffer, size_t length);

#endif
