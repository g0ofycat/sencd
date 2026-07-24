#ifndef SERVER_ENV_H
#define SERVER_ENV_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "utility/clear.h"

#include "../server/server_core.h"
#include "../server/serverinit/idle.h"

#include "../sessions/session_manager.h"

//--============
// -- CONSTS
//--============

#define SHELL_PREFIX "sencd-server > "

//--============
// -- TYPEDEFS
//--============

typedef void (*SERVER_COMMAND_FUNC)();

typedef struct { // TODO: actual parsing
	const char *command;
	SERVER_COMMAND_FUNC function;
} SERVER_COMMAND;

//--============
// -- DEFINITIONS
//--============

/// @brief start server term env and handle all commands
void start_server_environment(void);

#endif
