#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "session.h"

//--============
// -- CONSTS
//--============

#define MAX_SESSIONS 1024

//--============
// -- TYPEDEFS
//--============

typedef struct {
	SESSION_T *sessions[MAX_SESSIONS];
	uint32_t session_count;
	pthread_mutex_t lock;
} SESSION_MANAGER_T;

//--============
// -- DEFINITIONS
//--============

/// @brief initialize the session manager
/// @param *manager
void session_manager_init(SESSION_MANAGER_T *manager);

/// @brief create, handshake, and add a session
/// @param *manager
/// @param socket
/// @param *ip
/// @return SESSION_T*: NULL on failure
SESSION_T *session_manager_connect(SESSION_MANAGER_T *manager,
								   SESSION_ROLE_T role, int socket,
								   const char *ip);

/// @brief disconnect and remove a session
/// @param *manager
/// @param session_id
/// @return int: success bool
int session_manager_disconnect(SESSION_MANAGER_T *manager, uint64_t session_id);

/// @brief destroy the session manager and all active sessions
/// @param *manager
void session_manager_destroy(SESSION_MANAGER_T *manager);

/// @brief add a session to the manager
/// @param *manager
/// @param *session
/// @return int: success bool
int session_manager_add(SESSION_MANAGER_T *manager, SESSION_T *session);

/// @brief remove a session from the manager
/// @param *manager
/// @param session_id
/// @return int: success bool
int session_manager_remove(SESSION_MANAGER_T *manager, uint64_t session_id);

/// @brief find a session by its id
/// @param *manager
/// @param session_id
/// @return SESSION_T*: session or NULL
SESSION_T *session_manager_get(SESSION_MANAGER_T *manager, uint64_t session_id);

#endif
