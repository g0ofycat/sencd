#include "client_env.h"

//--============
// -- CONFIG
//--============

static CLIENT_COMMAND commands[] = {{"clear", cmd_clear}};

//--============
// -- PRIVATE
//--============

static const int command_count = sizeof(commands) / sizeof(commands[0]);

/// @brief exec func mapped to cmd
/// @param *input
static void execute_command(char *input) {
	for (int i = 0; i < command_count; i++) {
		if (strcmp(input, commands[i].command) == 0) {
			commands[i].function();
			return;
		}
	}

	log_msg(ERROR_MSG, CLIENT_RT, "Unknown command\n");
}

/// @brief thread for connecting and authenticating to the server
/// @param *arg
/// @param *ip
/// @param port
static void *client_listener(void *arg) {
	CLIENT_THREAD_DATA *data = (CLIENT_THREAD_DATA *)arg;

	if (connection_connect(data->conn, data->ip, data->port) == 0) {
		if (session_client_connect(data->conn) != 0) {
			log_msg(ERROR_MSG, CLIENT_RT, "Handshake Failed");
			data->connection_status = 1;
			return NULL;
		}
		log_msg(SUCCESS_MSG, CLIENT_RT, "Handshake Completed");
	} else {
		log_msg(ERROR_MSG, CLIENT_RT, "Connection Failed");
		data->connection_status = 1;
	}

	return NULL;
}

//--============
// -- LOGIC
//--============

void start_client_environment(int argc, char *argv[]) {
	CONNECTION_T conn;

	connection_init(&conn);

	pthread_t listener;

	CLIENT_THREAD_DATA data = {
		.conn = &conn,
		.argv = argv,
		.ip = CLIENT_DEFAULT_IP,
		.argc = argc,
		.port = CLIENT_DEFAULT_PORT,
		.connection_status = 0
	};

	if (argc > 2)
		data.ip = argv[2];

	if (argc > 3)
		data.port = (uint16_t)atoi(argv[3]);

	pthread_create(&listener, NULL, client_listener, &data);

	pthread_join(listener, NULL);

	if (data.connection_status != 0) {
		log_msg(ERROR_MSG, CLIENT_RT, "Exiting due to connection failure.\n");
		connection_disconnect(&conn);
		return;
	}

	char input[256];

	while (1) {
		printf(SHELL_PREFIX);
		fflush(stdout);

		if (fgets(input, sizeof(input), stdin) == NULL)
			break;

		input[strcspn(input, "\n")] = 0;
		execute_command(input);
	}

	connection_disconnect(&conn);
}
