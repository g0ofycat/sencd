#ifndef IDLE_H
#define IDLE_H

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

//--============
// -- CONSTS
//--============

#define EXIT_KEY 24

//--============
// -- DEFINITIONS
//--============

/// @brief allow for log_msg to pass through and makes term output only
void idle_mode(void);

/// @brief check if the term is currently idle
/// @return int: bool
int is_idle(void);

#endif
