#include "server_env.h"

//--============
// -- CONFIG
//--============

static SERVER_COMMAND commands[] = {{"idle", idle_mode}, {"clear", cmd_clear}};

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

	force_logs = 1;
	log_msg(ERROR_MSG, SERVER_RT, "Unknown command\n");
	force_logs = 0;
}

/// @brief thread for listening to clients
/// @param *arg
static void *server_listener(void *arg) {
	SERVER_LISTENER_DATA *listener = arg;

	while (listener->server->running) {
		int client = server_accept(listener->server);

		if (client < 0)
			continue;

		if (session_manager_connect(
					listener->session_manager,
					SESSION_SERVER,
					client,
					"127.0.0.1") == NULL) {

			close(client);
		}
	}

	return NULL;
}

//--============
// -- LOGIC
//--============

void start_server_environment(void) {
	SERVER_T server;
	server_init(&server);

	force_logs = 1;
	if (server_start(&server, SERVER_DEFAULT_PORT) != 0) {
		force_logs = 0;
		return;
	}
	force_logs = 0;

	SESSION_MANAGER_T session_manager;
	session_manager_init(&session_manager);

	SERVER_LISTENER_DATA listener_data = {
		.server = &server,
		.session_manager = &session_manager
	};

	pthread_t listener_thread;

	pthread_create(
			&listener_thread,
			NULL,
			server_listener,
			&listener_data
			);

	char input[256];

	while (1) {
		printf(SHELL_PREFIX);
		fflush(stdout);

		if (fgets(input, sizeof(input), stdin) == NULL)
			break;

		input[strcspn(input, "\n")] = 0;
		execute_command(input);
	}

	server_shutdown(&server);
	pthread_join(listener_thread, NULL);
	session_manager_destroy(&session_manager);
}
