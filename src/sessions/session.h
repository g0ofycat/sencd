#ifndef SESSION_H
#define SESSION_H

#include <arpa/inet.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>

#include "../shared/packet.h"
#include "../client/client_core.h"

//--============
// -- TYPEDEFS
//--============

typedef enum {
	SESSION_CONNECTING,
	SESSION_WAIT_AUTH,
	SESSION_AUTHENTICATED,
	SESSION_DISCONNECTED
} SESSION_STATE_T;

typedef enum {
	SESSION_SERVER,
	SESSION_CLIENT
} SESSION_ROLE_T; // TODO: i may use this for the encryption, compression, handshake, wtv.

typedef struct {
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
int session_create(SESSION_T *session, SESSION_ROLE_T role, int socket, const char *ip);

/// @brief handle the initial connection handshake [server]
/// @param *session
/// @return int: success bool
int session_server_connect(SESSION_T *session);

/// @brief handle the initial connection handshake [client]
/// @param *connection
/// @return int: success bool
int session_client_connect(CONNECTION_T *connection);

/// @brief authenticate the session
/// @param *session
/// @return int: success bool
int session_authenticate(SESSION_T *session);

/// @brief destroy a session
/// @param *session
void session_destroy(SESSION_T *session);

#endif
