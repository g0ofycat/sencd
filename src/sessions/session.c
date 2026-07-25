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

	crypto_context_init(&session->crypto);
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

void session_destroy(SESSION_T *session) {
	if (session->socket >= 0) {
		close(session->socket);
	}

	memset(session, 0, sizeof(*session));

	session->socket = -1;
	session->state = SESSION_DISCONNECTED;
}

int session_authenticate_server(SESSION_T *session) {
	AUTH_CHALLENGE_T challenge;
	memset(&challenge, 0, sizeof(challenge));

	crypto_generate_nonce(session->auth_nonce, AUTH_NONCE_SIZE);

	memcpy(challenge.public_key,
			session->crypto.public_key,
			CRYPTO_PUBLIC_KEY_SIZE);

	memcpy(challenge.nonce,
			session->auth_nonce,
			AUTH_NONCE_SIZE);

	PACKET packet = packet_construct(
			(PACKET_CONSTRUCTOR_T){
			.header_type = PACKET_AUTH_REQUEST,
			.header_version = session->protocol_version,
			.flags = PACKET_FLAG_NONE,
			.payload = (uint8_t *)&challenge,
			.payload_length = sizeof(AUTH_CHALLENGE_T)
			}
			);

	if (packet_send(session->socket, &packet) != 0) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to send the authentication request packet");
		packet_destroy(&packet);
		return 1;
	}

	packet_destroy(&packet);

	session->state = SESSION_WAIT_AUTH;

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
			.header_version = session->protocol_version});

	if (packet_send(session->socket, &hello) != 0) {
		packet_destroy(&hello);
		return 1;
	}

	packet_destroy(&hello);

	session->state = SESSION_WAIT_AUTH;

	log_msg(SUCCESS_MSG, SERVER_RT, "Handshake completed");

	return 0;
}

int session_authenticate_client(SESSION_T *session) {
	PACKET packet;
	packet_init(&packet);

	if (packet_receive(session->socket, &packet) != 0) {
		packet_destroy(&packet);
		return 1;
	}

	if (packet.header.type != PACKET_AUTH_REQUEST) {
		packet_destroy(&packet);
		return 1;
	}

	if (packet.header.payload_length != sizeof(AUTH_CHALLENGE_T)) {
		packet_destroy(&packet);
		return 1;
	}

	AUTH_CHALLENGE_T challenge;
	memcpy(&challenge, packet.payload, sizeof(challenge));

	packet_destroy(&packet);

	memcpy(session->crypto.peer_public_key,
			challenge.public_key,
			CRYPTO_PUBLIC_KEY_SIZE);

	if (crypto_generate_identity(&session->crypto) != 0) {
		return 1;
	}

	AUTH_RESPONSE_T response;
	memset(&response, 0, sizeof(response));

	memcpy(response.public_key,
			session->crypto.public_key,
			CRYPTO_PUBLIC_KEY_SIZE);

	if (crypto_sign_message(
				&session->crypto,
				challenge.nonce,
				AUTH_NONCE_SIZE,
				response.signature) != 0) {
		return 1;
	}

	PACKET reply = packet_construct(
			(PACKET_CONSTRUCTOR_T){
			.header_type = PACKET_AUTH_RESPONSE,
			.header_version = session->protocol_version,
			.flags = PACKET_FLAG_NONE,
			.payload = (uint8_t *)&response,
			.payload_length = sizeof(AUTH_RESPONSE_T)
			}
			);

	if (packet_send(session->socket, &reply) != 0) {
		log_msg(ERROR_MSG, CLIENT_RT, "Failed to send the authentication response packet");
		packet_destroy(&reply);
		return 1;
	}

	packet_destroy(&reply);

	return 0;
}

int session_client_connect(CONNECTION_T *connection) {
	PACKET hello = packet_construct((PACKET_CONSTRUCTOR_T){
			.header_type = PACKET_CLIENT_HELLO, .header_version = 1});

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

