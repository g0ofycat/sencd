#include "client_env.h"

//--============
// -- COMMANDS
//--============

static CLIENT_T *active_client = NULL;
static volatile int should_exit = 0;

static void cmd_disconnect(char **argv) {
	if (active_client == NULL || active_client->session.state != SESSION_ESTABLISHED) {
		log_msg(WARN_MSG, CLIENT_RT, "Not currently connected");
		return;
	}

	session_destroy(&active_client->session);
	active_client->connection.socket = -1;
	active_client->connection.state = CONNECTION_DISCONNECTED;

	log_msg(SUCCESS_MSG, CLIENT_RT, "Disconnected from server");
	should_exit = 1;
}

//--============
// -- CONFIG
//--============

static CLIENT_COMMAND commands[] = {{"clear", NULL, cmd_clear}, {"disconnect", NULL, cmd_disconnect}};

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

/// @brief recv packets from the server
/// @param *arg
static void *client_receiver(void *arg) {
	CLIENT_T *client = (CLIENT_T *)arg;

	while (client->session.state == SESSION_ESTABLISHED) {
		PACKET packet;
		packet_init(&packet);

		if (packet_receive(client->session.socket, &packet) != 0) {
			packet_destroy(&packet);
			break;
		}

		if (packet.header.type == PACKET_DISCONNECT) {
		packet_destroy(&packet);
		log_msg(WARN_MSG, CLIENT_RT, "Server closed the connection");
		break;
	}

		packet_destroy(&packet);
	}

	if (client->session.state == SESSION_ESTABLISHED) {
		session_destroy(&client->session);
		client->connection.socket = -1;
		client->connection.state = CONNECTION_DISCONNECTED;
	}

	should_exit = 1;
	return NULL;
}

/// @brief recv network packets from the os to client
/// @param *arg
static void *client_udp_receiver(void *arg) {
	CLIENT_T *client = (CLIENT_T *)arg;
	uint8_t *buffer = malloc(65536);
	if (buffer == NULL) {
		log_msg(ERROR_MSG, CLIENT_RT, "Failed to allocate UDP receive buffer");
		return NULL;
	}

	while (client->session.state == SESSION_ESTABLISHED && client->session.udp.socket >= 0) {
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(client->session.udp.socket, &readfds);
		struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};

		int ready = select(client->session.udp.socket + 1, &readfds, NULL, NULL, &timeout);

		if (client->session.state != SESSION_ESTABLISHED || client->session.udp.socket < 0)
			break;
		if (ready <= 0)
			continue;

		ssize_t received = recv(client->session.udp.socket, buffer, 65536, 0);
		if (received < (ssize_t)sizeof(uint64_t))
			continue;
	}

	free(buffer);
	return NULL;
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
		int has_trusted = crypto_config_path("server_trusted.key", path, sizeof(path)) == 0 &&
			crypto_trust_key_load(path, trusted_key) == 0;

		session_create(
				&data->client->session,
				SESSION_CLIENT,
				data->client->connection.socket,
				data->ip,
				&data->client->identity,
				has_trusted ? trusted_key : NULL);

		if (session_client_connect(&data->client->session) == 0) {
			if (udp_tunnel_client_open(&data->client->session.udp, data->ip, data->port) == 0) {
				pthread_t udp_receiver;
				pthread_create(&udp_receiver, NULL, client_udp_receiver, data->client);
				pthread_detach(udp_receiver);
			}
		}
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
	active_client = &client;
	connection_init(&client.connection);
	session_init(&client.session, SESSION_CLIENT);

	memset(&client.identity, 0, sizeof(client.identity));
	char identity_path[PATH_MAX];
	if (crypto_config_path("client_identity.key", identity_path, sizeof(identity_path)) != 0 ||
			crypto_identity_load_or_create(&client.identity, identity_path) != 0) {
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

	pthread_t receiver;
	pthread_create(&receiver, NULL, client_receiver, &client);
	pthread_detach(receiver);

	char input[256];

	printf(SHELL_PREFIX);
	fflush(stdout);

	while (!should_exit) {
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(STDIN_FILENO, &readfds);
		struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};

		int ready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

		if (should_exit)
			break;

		if (ready <= 0)
			continue;

		if (fgets(input, sizeof(input), stdin) == NULL)
			break;

		input[strcspn(input, "\n")] = 0;
		execute_command(input);

		if (should_exit)
			break;

		printf(SHELL_PREFIX);
		fflush(stdout);
	}

	if (client.connection.state != CONNECTION_DISCONNECTED)
		connection_disconnect(&client.connection);
}
