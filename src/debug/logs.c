#include "logs.h"

#include "../server/serverinit/idle.h"

//--============
// -- CONFIG
//--============

int force_logs = 0;

//--============
// -- LOGIC
//--============

void log_msg(MSG_T message_t, RUNTIME_T runtime_t, char* message, ...) {
	char* fmt_message = NULL;

	va_list args;
	va_start(args, message);

	va_list args_copy;
	va_copy(args_copy, args);
	int len = vsnprintf(NULL, 0, message, args_copy);
	va_end(args_copy);

	if (len >= 0) {
		fmt_message = malloc(len + 1);
		if (fmt_message) {
			vsnprintf(fmt_message, len + 1, message, args);
		}
	}

	va_end(args);

	if (!fmt_message) {
		printf("'log_msg' failed to format the string");
		return;
	}

	if (runtime_t == SERVER_RT && !is_idle() && !force_logs)
		return;

	const char *asterisk_color;
	switch (message_t) {
		case INFO_MSG:
			asterisk_color = BLUE;
			break;
		case SUCCESS_MSG:
			asterisk_color = GREEN;
			break;
		case WARN_MSG:
			asterisk_color = YELLOW;
			break;
		case ERROR_MSG:
			asterisk_color = RED;
			break;
		default:
			asterisk_color = WHITE;
	}

	const char *runtime_label;
	switch (runtime_t) {
		case SERVER_RT:
			runtime_label = "SERVER";
			break;
		case CLIENT_RT:
			runtime_label = "CLIENT";
			break;
		case OTHER_RT:
			runtime_label = "OTHER";
			break;
		default:
			runtime_label = "UNKNOWN";
	}

	printf("[%s*%s] [%s]: %s\n", asterisk_color, RESET, runtime_label, fmt_message);
	fflush(stdout);
}
