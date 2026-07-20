#include "server_env.h"

//--============
// -- CONFIG
//--============

static SERVER_COMMAND commands[] = {
	{
		"idle",
		idle_mode
	}
};

//--============
// -- PRIVATE
//--============

static int command_count = sizeof(commands) / sizeof(commands[0]);

/// @brief exec func mapped to cmd
/// @param *input
static void execute_command(char* input)
{
	for(int i = 0; i < command_count; i++) {
		if(strcmp(input, commands[i].command) == 0) {
			commands[i].function();
			return;
		}
	}

	log_msg(ERROR_MSG, SERVER_RT, "Unknown command\n");
}

/// @brief thread for listening to clients
/// @param *arg
static void* server_listener(void* arg) {
	SERVER_T* server = (SERVER_T*)arg;

	while(server->running)
	{
		int client = server_accept(server);
		if(client >= 0)
		{
			close(client);
		}
	}

	return NULL;
}

//--============
// -- LOGIC
//--============

void start_server_environment()
{
	SERVER_T server;

	force_logs = 1;

	server_init(&server);
	server_start(&server, SERVER_DEFAULT_PORT);

	force_logs = 0;

	pthread_t listener;

	pthread_create(
			&listener,
			NULL,
			server_listener,
			&server
			);

	char input[256];

	while(1) {
		printf(SHELL_PREFIX);
		fflush(stdout);

		if(fgets(input, sizeof(input), stdin) == NULL)
			break;

		input[strcspn(input, "\n")] = 0;
		execute_command(input);
	}

	server_shutdown(&server);
	pthread_join(listener, NULL);
}
