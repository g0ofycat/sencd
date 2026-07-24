#include "session.h"

//--============
// -- PRIVATE
//--============

static uint64_t next_session_id = 0;

/// @brief generate id
/// @return uint64_t: incrementing int
static uint64_t generate_session_id() { return next_session_id++; }

//--============
// -- LOGIC
//--============

void session_init(SESSION_T *session, SESSION_ROLE_T role) {
	memset(session, 0, sizeof(*session));
	session->socket = -1;
	session->state = SESSION_DISCONNECTED;
	session->role = role;
}

int session_create(SESSION_T *session, SESSION_ROLE_T role, int socket,
				   const char *ip) {
	session_init(session, role);

	session->socket = socket;
	session->session_id = generate_session_id();

	session->created = time(NULL);
	session->last_seen = session->created;

	session->protocol_version = 1;

	strncpy(session->ip, ip, sizeof(session->ip) - 1);

	session->state = SESSION_CONNECTING;

	return 0;
}

int session_server_connect(SESSION_T *session) {
	PACKET packet;

	packet_init(&packet);

	if (packet_receive(session->socket, &packet) != 0) {
		packet_destroy(&packet);
		return 1;
	}

	if (packet.header.type != PACKET_CLIENT_HELLO) {
		packet_destroy(&packet);
		return 1;
	}

	packet_destroy(&packet);

	PACKET hello = packet_construct(
		(PACKET_CONSTRUCTOR_T){.header_type = PACKET_SERVER_HELLO,
							   .header_version = session->protocol_version,
							   .message = ""});

	if (packet_send(session->socket, &hello) != 0) {
		packet_destroy(&hello);
		return 1;
	}

	packet_destroy(&hello);

	session->state = SESSION_WAIT_AUTH;

	log_msg(SUCCESS_MSG, SERVER_RT, "Handshake completed");

	return 0;
}

int session_client_connect(CONNECTION_T *connection) {
	PACKET hello = packet_construct(
		(PACKET_CONSTRUCTOR_T){.header_type = PACKET_CLIENT_HELLO,
							   .header_version = 1,
							   .message = ""});

	packet_send(connection->socket, &hello);

	packet_destroy(&hello);

	PACKET response;
	packet_init(&response);

	if (packet_receive(connection->socket, &response) == 1) {
		packet_destroy(&response);
		return 1;
	}

	packet_destroy(&response);

	return 0;
}

void session_destroy(SESSION_T *session) {
	if (session->socket >= 0) {
		close(session->socket);
	}

	memset(session, 0, sizeof(*session));

	session->socket = -1;
	session->state = SESSION_DISCONNECTED;
}
