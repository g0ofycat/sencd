// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/commands/server_env.h: Console environment for the server
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

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

#define SERVER_DEFAULT_IP "127.0.0.1"

//--============
// -- TYPEDEFS
//--============

typedef void (*SERVER_COMMAND_FUNC)(char **arg);

typedef struct {
	const char *command;
	char **argv;
	SERVER_COMMAND_FUNC function;
} SERVER_COMMAND;

typedef struct {
	SERVER_T *server;
	SESSION_MANAGER_T *session_manager;
	const char *ip;
} SERVER_LISTENER_DATA;

//--============
// -- DEFINITIONS
//--============

/// @brief start server term env and handle all commands
/// @param *ip
void start_server_environment(int argc, char *argv[]);

#endif
