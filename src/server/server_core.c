#include "server_core.h"

//--============
// -- FORWARD DECL.
//--============

/// @brief create TCP socket for clients to attach to
/// @param *server
/// @return int: success bool
static int server_create_socket(SERVER_T *server);

/// @brief bind socket to server
/// @param *server
/// @return int: success bool
static int server_bind(SERVER_T *server);

/// @brief listen to any connections
/// @param *server
/// @return int: success bool
static int server_listen(SERVER_T *server);

/// @brief socket cleanup on failure
/// @param *server
static void server_cleanup(SERVER_T *server)
{
	if (server->socket >= 0)
		close(server->socket);

	server->socket = -1;
	server->running = 0;
}

//--============
// -- LOGIC
//--============

void server_init(SERVER_T *server)
{
	memset(server, 0, sizeof(*server));

	server->socket = -1;
	server->running = 0;
}

int server_start(SERVER_T *server, uint16_t port)
{
	if (server->running) {
		log_msg(ERROR_MSG, SERVER_RT, "Server is already running");
		return 1;
	}

	log_msg(INFO_MSG, SERVER_RT, "Starting server...");

	server->port = port;

	if (server_create_socket(server)) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to create listening socket");
		return 1;
	}

	if (server_bind(server)) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to bind server socket");
		return 1;
	}

	if (server_listen(server)) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to listen for connections");
		return 1;
	}

	server->running = 1;

	char buffer[64];

	snprintf(
			buffer,
			sizeof(buffer),
			"Listening on port %u",
			server->port
			);

	log_msg(INFO_MSG, SERVER_RT, buffer);

	return 0;
}

int server_accept(SERVER_T *server)
{
	struct sockaddr_in client_addr = {0};
	socklen_t client_len = sizeof(client_addr);

	int client_socket = accept(
			server->socket,
			(struct sockaddr *)&client_addr,
			&client_len
			);

	if (client_socket < 0) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to accept client");
		perror("accept");
		return -1;
	}

	char client_ip[INET_ADDRSTRLEN];

	if (inet_ntop(
				AF_INET,
				&client_addr.sin_addr,
				client_ip,
				sizeof(client_ip)) == NULL)
	{
		strcpy(client_ip, "<unknown>");
	}

	server->clients_connected++;

	log_msg(INFO_MSG, SERVER_RT, "Client connected");

	printf(
			"Address: %s\n"
			"Port: %u\n",
			client_ip,
			ntohs(client_addr.sin_port)
		  );

	return client_socket;
}

int server_shutdown(SERVER_T *server)
{
	if (!server->running) {
		log_msg(ERROR_MSG, SERVER_RT, "Server is not running");
		return 1;
	}

	log_msg(INFO_MSG, SERVER_RT, "Shutting down server...");

	server_cleanup(server);

	log_msg(INFO_MSG, SERVER_RT, "Server shutdown complete");

	return 0;
}

//--============
// -- INTERNAL
//--============

static int server_create_socket(SERVER_T *server)
{
	server->socket = socket(AF_INET, SOCK_STREAM, 0);

	if (server->socket < 0) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to create socket");
		perror("socket");

		server_cleanup(server);

		return 1;
	}

	int opt = 1;

	if (setsockopt(
				server->socket,
				SOL_SOCKET,
				SO_REUSEADDR,
				&opt,
				sizeof(opt)) < 0)
	{
		log_msg(ERROR_MSG, SERVER_RT, "Failed to set SO_REUSEADDR");
		perror("setsockopt");

		server_cleanup(server);

		return 1;
	}

	return 0;
}

static int server_bind(SERVER_T *server)
{
	struct sockaddr_in server_addr = {0};

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(server->port);

	if (bind(
				server->socket,
				(struct sockaddr *)&server_addr,
				sizeof(server_addr)) < 0)
	{
		log_msg(ERROR_MSG, SERVER_RT, "Failed to bind socket");
		perror("bind");

		server_cleanup(server);

		return 1;
	}

	return 0;
}

static int server_listen(SERVER_T *server)
{
	if (listen(server->socket, SOMAXCONN) < 0) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to listen");
		perror("listen");

		server_cleanup(server);

		return 1;
	}

	return 0;
}
