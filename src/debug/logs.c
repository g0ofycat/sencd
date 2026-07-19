#include "logs.h"

//--============
// -- LOGIC
//--============

void log_msg(MSG_T message_t, RUNTIME_T runtime_t, char* message) {
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

	printf("[%s*%s] [%s]: %s\n", asterisk_color, RESET, runtime_label, message);
}
