#ifndef LOGS_H
#define LOGS_H

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

//--============
// -- CONFIG
//--============

extern int force_logs;

//--============
// -- TYPEDEFS
//--============

typedef enum { INFO_MSG, SUCCESS_MSG, WARN_MSG, ERROR_MSG } MSG_T;

typedef enum { SERVER_RT, CLIENT_RT, OTHER_RT } RUNTIME_T;

//--============
// -- CONSTS
//--============

#define WHITE "\033[97m"
#define BLUE "\033[94m"
#define GREEN "\033[32m"
#define YELLOW "\033[93m"
#define RED "\033[31m"
#define RESET "\033[0m"

//--============
// -- DEFINITIONS
//--============

/// @brief debug func for all logs
/// @param message_t: message type to display
/// @param runtime_t: runtime exec env
/// @param message: info to display, can be a formatted string
/// @varadic: string data to format
void log_msg(MSG_T message_t, RUNTIME_T runtime_t, char *message, ...);

#endif
