#include "session.h"

//--============
// -- PRIVATE
//--============

/// @brief generate id
/// @return uint64_t: random bytes
static uint64_t generate_session_id() {
	uint64_t id;
	randombytes_buf(&id, sizeof(id));
	return id;
}

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

	session->protocol_version = CURRENT_PROTOCOL_VERSION;

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

	if (packet.header.version != CURRENT_PROTOCOL_VERSION) {
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

	if (session_authenticate_server(session) != 0) {
		log_msg(ERROR_MSG, SERVER_RT, "Couldn't authenticate the server");
		return 1;
	}

	if (session_verify_client(session) != 0) {
		log_msg(ERROR_MSG, SERVER_RT, "Couldn't verify the current session");
		return 1;
	}

	crypto_generate_nonce(
			session->session_nonce,
			SESSION_NONCE_SIZE
			);

	AUTH_SUCCESS_T success_data;
	memset(&success_data, 0, sizeof(success_data));

	success_data.session_id = session->session_id;

	memcpy(
			success_data.session_nonce,
			session->session_nonce,
			SESSION_NONCE_SIZE
		  );

	PACKET success = packet_construct(
			(PACKET_CONSTRUCTOR_T){
			.header_type = PACKET_AUTH_SUCCESS,
			.header_version = session->protocol_version,
			.payload = (uint8_t *)&success_data,
			.payload_length = sizeof(success_data)
			}
			);

	if (packet_send(session->socket, &success) != 0) {
		packet_destroy(&success);
		return 1;
	}

	packet_destroy(&success);

	session->state = SESSION_ESTABLISHED;

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

	if (memcmp(
				challenge.public_key,
				TRUSTED_SERVER_KEY,
				CRYPTO_PUBLIC_KEY_SIZE) != 0) {
		return 1;
	}

	memcpy(
			session->crypto.peer_public_key,
			challenge.public_key,
			CRYPTO_PUBLIC_KEY_SIZE
		  );

	AUTH_RESPONSE_T response;
	memset(&response, 0, sizeof(response));

	memcpy(
			response.public_key,
			session->crypto.public_key,
			CRYPTO_PUBLIC_KEY_SIZE
		  );

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

	session->state = SESSION_WAIT_AUTH;

	PACKET result;
	packet_init(&result);

	if (packet_receive(session->socket, &result) != 0) {
		packet_destroy(&result);
		return 1;
	}

	if (result.header.type != PACKET_AUTH_SUCCESS) {
		packet_destroy(&result);
		return 1;
	}

	if (result.header.payload_length != sizeof(AUTH_SUCCESS_T)) {
		packet_destroy(&result);
		return 1;
	}

	AUTH_SUCCESS_T success;

	memcpy(
			&success,
			result.payload,
			sizeof(success)
		  );

	packet_destroy(&result);

	session->session_id = success.session_id;

	memcpy(
			session->session_nonce,
			success.session_nonce,
			SESSION_NONCE_SIZE
		  );

	session->state = SESSION_ESTABLISHED;

	return 0;
}

int session_client_connect(SESSION_T *session) {
	PACKET hello = packet_construct((PACKET_CONSTRUCTOR_T){
			.header_type = PACKET_CLIENT_HELLO, .header_version = CURRENT_PROTOCOL_VERSION});

	if (packet_send(session->socket, &hello) != 0) {
		packet_destroy(&hello);
		return 1;
	}

	packet_destroy(&hello);

	PACKET response;
	packet_init(&response);

	if (packet_receive(session->socket, &response) != 0) {
		packet_destroy(&response);
		return 1;
	}

	if (response.header.type != PACKET_SERVER_HELLO) {
		packet_destroy(&response);
		return 1;
	}

	if (response.header.version != CURRENT_PROTOCOL_VERSION) {
		packet_destroy(&response);
		return 1;
	}

	packet_destroy(&response);

	return session_authenticate_client(session);
}

int session_verify_client(SESSION_T *session) {
	PACKET packet;
	packet_init(&packet);

	if (packet_receive(session->socket, &packet) != 0) {
		packet_destroy(&packet);
		return 1;
	}

	if (packet.header.type != PACKET_AUTH_RESPONSE) {
		packet_destroy(&packet);
		return 1;
	}

	AUTH_RESPONSE_T response;

	if (packet.header.payload_length != sizeof(AUTH_RESPONSE_T)) {
		packet_destroy(&packet);
		return 1;
	}

	memcpy(
			&response,
			packet.payload,
			sizeof(response)
		  );

	packet_destroy(&packet);

	memcpy(
			session->crypto.peer_public_key,
			response.public_key,
			CRYPTO_PUBLIC_KEY_SIZE
		  );

	if (memcmp(
				response.public_key,
				TRUSTED_SERVER_KEY,
				CRYPTO_PUBLIC_KEY_SIZE
			  ) != 0) {
		return 1;
	}

	if (crypto_verify_message(
				response.public_key,
				session->auth_nonce,
				AUTH_NONCE_SIZE,
				response.signature)) {
		return 1;
	}

	session->crypto.verified = 1;

	return 0;
}
