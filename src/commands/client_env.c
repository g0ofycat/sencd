#include "client_env.h"

//--============
// -- CONFIG
//--============

static CLIENT_COMMAND commands[] = {
	{
		"clear",
		cmd_clear
	}
};

//--============
// -- PRIVATE
//--============

static const int command_count = sizeof(commands) / sizeof(commands[0]);

/// @brief exec func mapped to cmd
/// @param *input
static void execute_command(char* input) {
	for(int i = 0; i < command_count; i++) {
		if(strcmp(input, commands[i].command) == 0) {
			commands[i].function();
			return;
		}
	}

	log_msg(ERROR_MSG, CLIENT_RT, "Unknown command\n");
}

/// @brief thread for listening to the server
/// @param *arg
static void* client_listener(void* arg) {
	CONNECTION_T* conn = (CONNECTION_T*)arg;

	if (connection_connect(conn, CLIENT_DEFAULT_IP, CLIENT_DEFAULT_PORT) == 0) {
		if (session_client_connect(conn) == 0) {
			log_msg(SUCCESS_MSG, CLIENT_RT, "Handshake Completed");
		}
	}

	return NULL;
}

//--============
// -- LOGIC
//--============

void start_client_environment(int argc, char* argv[]) {
	CONNECTION_T conn;

	connection_init(&conn);

	pthread_t listener;

	CLIENT_THREAD_DATA data = {
		.conn = &conn,
		.argc = argc,
		.argv = argv
	};

	pthread_create(&listener, NULL, client_listener, &data);
	pthread_join(listener, NULL);

	char input[256];

	while(1) {
		printf(SHELL_PREFIX);
		fflush(stdout);

		if(fgets(input, sizeof(input), stdin) == NULL)
			break;

		input[strcspn(input, "\n")] = 0;
		execute_command(input);
	}

	connection_disconnect(&conn);
}
