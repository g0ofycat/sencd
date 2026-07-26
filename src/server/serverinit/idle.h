// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/server/serverinit/idle.h: Read-only mode for terminal
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#ifndef IDLE_H
#define IDLE_H

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#include "../../commands/utility/clear.h"

//--============
// -- CONSTS
//--============

#define EXIT_KEY 24

//--============
// -- DEFINITIONS
//--============

/// @brief allow for log_msg to pass through and makes term output only
/// @param **argv: unused
void idle_mode(char **argv);

/// @brief check if the term is currently idle
/// @return int: bool
int is_idle(void);

#endif
