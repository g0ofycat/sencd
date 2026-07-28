// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/commands/client_env.h: Console environment for the client
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#ifndef CLIENT_ENV_H
#define CLIENT_ENV_H

#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "utility/clear.h"

#include "../client/client_core.h"
#include "../debug/logs.h"

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

typedef struct {
	const char *command;
	char **argv;
	CLIENT_COMMAND_FUNC function;
} CLIENT_COMMAND;

typedef struct {
	CONNECTION_T connection;
	SESSION_T session;
	CRYPTO_CONTEXT_T identity;
} CLIENT_T;

typedef struct {
	CLIENT_T *client;
	char **argv;
	const char *ip;
	int argc;
	int connection_status;
	uint16_t port;
} CLIENT_THREAD_DATA;

//--============
// -- DEFINITIONS
//--============

/// @brief start client term env and handle all commands
/// @param *argv
void start_client_environment(int argc, char *argv[]);

#endif
