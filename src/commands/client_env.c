#include "client_env.h"

//--============
// -- CONFIG
//--============

static CLIENT_COMMAND commands[] = {{"clear", NULL, cmd_clear}};

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
	if (connection_connect(&data->client->connection, data->ip, data->port) == 0) {
		unsigned char trusted_key[CRYPTO_PUBLIC_KEY_SIZE];
		char path[PATH_MAX];
		int has_trusted = (crypto_config_path("server_trusted.key", path, sizeof(path)) == 0 &&
				crypto_trust_key_load(path, trusted_key) == 0);

		session_create(
				&data->client->session,
				SESSION_CLIENT,
				data->client->connection.socket,
				data->ip,
				&data->client->identity,
				has_trusted ? trusted_key : NULL);

		if (session_client_connect(&data->client->session) != 0) {
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
	CLIENT_T client;
	connection_init(&client.connection);
	session_init(&client.session, SESSION_CLIENT);

	memset(&client.identity, 0, sizeof(client.identity));
	if (crypto_identity_load_or_create(&client.identity, "client_identity.key") != 0) {
		log_msg(ERROR_MSG, CLIENT_RT, "Failed to load or create client identity");
		return;
	}

	pthread_t listener;

	CLIENT_THREAD_DATA data = {
		.client = &client,
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

	if (data.connection_status != 0)
		return;

	char input[256];

	while (1) {
		printf(SHELL_PREFIX);
		fflush(stdout);

		if (fgets(input, sizeof(input), stdin) == NULL)
			break;

		input[strcspn(input, "\n")] = 0;
		execute_command(input);
	}

	connection_disconnect(&client.connection);
}
