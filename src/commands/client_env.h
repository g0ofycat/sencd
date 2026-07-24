#ifndef CLIENT_ENV_H
#define CLIENT_ENV_H

#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include "utility/clear.h"

#include "../debug/logs.h"
#include "../client/client_core.h"
#include "../sessions/session.h"

//--============
// -- CONSTS
//--============

#define SHELL_PREFIX "sencd-client > "

#define CLIENT_DEFAULT_IP "127.0.0.1"

//--============
// -- TYPEDEFS
//--============

typedef void (*CLIENT_COMMAND_FUNC)();

typedef struct { // TODO: actual parsing
	const char* command;
	CLIENT_COMMAND_FUNC function;
} CLIENT_COMMAND;

typedef struct {
	CONNECTION_T* conn;
	int argc;
	char** argv;
} CLIENT_THREAD_DATA;

//--============
// -- DEFINITIONS
//--============

/// @brief start client term env and handle all commands
/// @param *argv
void start_client_environment(int argc, char* argv[]);

#endif
