#include "core.h"

//--============
// -- PRIVATE
//--============

/// @brief create a TCP socket on the connection
/// @param *connection
/// @return int: success bool
static int connection_create_socket(CONNECTION_T *connection);

/// @brief open a connection on the given port
/// @param *connection
/// @return int: success bool
static int connection_open(CONNECTION_T *connection);

/// @brief cleanup on error
/// @param *connection
static void connection_cleanup(CONNECTION_T *connection)
{
	if (connection->socket >= 0)
		close(connection->socket);

	connection->socket = -1;
	connection->state = CONNECTION_DISCONNECTED;
}

//--============
// -- LOGIC
//--============

void connection_init(CONNECTION_T *connection) {
	memset(connection, 0, sizeof(*connection));
	connection->socket = -1;
	connection->state = CONNECTION_DISCONNECTED;
}

int connection_connect(CONNECTION_T *connection, const char *ip, uint16_t port) {
	if (connection->state == CONNECTION_CONNECTED) { // TODO: possibly change this to per port
		log_msg(ERROR_MSG, CLIENT_RT, "Socket is already connected, aborting...");
		return 1;
	}

	log_msg(INFO_MSG, CLIENT_RT, "Attempting to connect to server...");

	strncpy(connection->ip, ip, sizeof(connection->ip) - 1);
	connection->ip[sizeof(connection->ip) - 1] = '\0';
	connection->port = port;
	connection->state = CONNECTION_CONNECTING;

	if (connection_create_socket(connection)) {
		log_msg(ERROR_MSG, CLIENT_RT, "Socket hasn't been created yet, aborting...");
		return 1;
	}
	if (connection_open(connection)) {
		log_msg(ERROR_MSG, CLIENT_RT, "Failed to open the connection");
		return 1;
	}

	connection->state = CONNECTION_CONNECTED;

	return 0;
};

int connection_disconnect(CONNECTION_T *connection)
{
	if (connection->state == CONNECTION_DISCONNECTED) {
		log_msg(ERROR_MSG, CLIENT_RT, "Socket is already disconnected");
		return 1;
	}

	log_msg(INFO_MSG, CLIENT_RT, "Disconnecting from server...");
	connection->state = CONNECTION_DISCONNECTING;
	connection_cleanup(connection);

	log_msg(INFO_MSG, CLIENT_RT, "Disconnected successfully");

	return 0;
}

ssize_t connection_send(CONNECTION_T *connection, const void *buffer, size_t length) {
	ssize_t sent = send(connection->socket, buffer, length, 0);

	if (sent > 0) {
		connection->bytes_sent += sent;
		connection->packets_sent++;
	}

	return sent;
}

ssize_t connection_receive(CONNECTION_T *connection, void *buffer, size_t length) {
	ssize_t rec = recv(connection->socket, buffer, length, 0);

	if (rec > 0) {
		connection->bytes_received += rec;
		connection->packets_received++;
	}

	return rec;
}

static int connection_create_socket(CONNECTION_T *connection) {
	connection->socket = socket(AF_INET, SOCK_STREAM, 0);

	if (connection->socket < 0) {
		log_msg(ERROR_MSG, CLIENT_RT, "Socket failed to create. Error:");
		perror("socket");

		connection_cleanup(connection);

		return 1;
	}

	return 0;
};

static int connection_open(CONNECTION_T *connection) {
	struct sockaddr_in server = {0};
	server.sin_family = AF_INET;
	server.sin_port = htons(connection->port);

	if (inet_pton(AF_INET, connection->ip, &server.sin_addr) != 1) {
		log_msg(ERROR_MSG, CLIENT_RT, "Couldn't resolve hostname");

		connection_cleanup(connection);

		return 1;
	}

	if (connect(connection->socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
		log_msg(ERROR_MSG, CLIENT_RT, "Connection issue. Error:");
		perror("connect");

		connection_cleanup(connection);

		return 1;
	}

	return 0;
}
