#include "session_manager.h"

//--============
// -- LOGIC
//--============

void session_manager_init(SESSION_MANAGER_T *manager) {
	memset(manager->sessions, 0, sizeof(manager->sessions));
	manager->session_count = 0;
	pthread_mutex_init(&manager->lock, NULL);
	manager->udp_socket = -1;
	memset(&manager->identity, 0, sizeof(manager->identity));

	char path[PATH_MAX];
	if (crypto_config_path("server_identity.key", path, sizeof(path)) != 0 ||
			crypto_identity_load_or_create(&manager->identity, path) != 0) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to load or create server identity");
	}
}

SESSION_T *session_manager_connect(SESSION_MANAGER_T *manager,
		SESSION_ROLE_T role, int socket,
		const char *ip) {
	SESSION_T *session = malloc(sizeof(*session));

	if (session == NULL)
		return NULL;

	if (session_create(session, role, socket, ip, &manager->identity, NULL) != 0) {
		free(session);
		return NULL;
	}

	if (session_server_connect(session) != 0) {
		session_destroy(session);
		free(session);
		return NULL;
	}

	if (session_manager_add(manager, session) != 0) {
		session_destroy(session);
		free(session);
		return NULL;
	}

	return session;
}

int session_manager_disconnect(SESSION_MANAGER_T *manager,
		uint64_t session_id) {
	return session_manager_remove(manager, session_id);
}

void session_manager_disconnect_all(SESSION_MANAGER_T *manager) {
	pthread_mutex_lock(&manager->lock);

	for (uint32_t i = 0; i < MAX_SESSIONS; i++) {
		SESSION_T *session = manager->sessions[i];
		if (session != NULL) {
			PACKET notice = packet_construct(
					(PACKET_CONSTRUCTOR_T){
					.header_type = PACKET_DISCONNECT,
					.header_version = session->protocol_version,
					.flags = PACKET_FLAG_NONE,
					.payload = NULL,
					.payload_length = 0
					}
					);

			packet_send(session->socket, &notice);
			packet_destroy(&notice);

			session_destroy(session);
			free(session);
			manager->sessions[i] = NULL;
		}
	}

	manager->session_count = 0;
	pthread_mutex_unlock(&manager->lock);
}

int session_manager_add(SESSION_MANAGER_T *manager, SESSION_T *session) {
	pthread_mutex_lock(&manager->lock);

	for (uint32_t i = 0; i < MAX_SESSIONS; i++) {
		if (manager->sessions[i] == NULL) {
			manager->sessions[i] = session;
			manager->session_count++;

			pthread_mutex_unlock(&manager->lock);
			return 0;
		}
	}

	pthread_mutex_unlock(&manager->lock);

	return 1;
}

int session_manager_remove(SESSION_MANAGER_T *manager, uint64_t session_id) {
	pthread_mutex_lock(&manager->lock);

	for (uint32_t i = 0; i < MAX_SESSIONS; i++) {
		SESSION_T *session = manager->sessions[i];
		if (session != NULL && session->session_id == session_id) {
			session_destroy(session);
			free(session);

			manager->sessions[i] = NULL;
			manager->session_count--;

			pthread_mutex_unlock(&manager->lock);
			return 0;
		}
	}

	pthread_mutex_unlock(&manager->lock);

	return 1;
}

SESSION_T *session_manager_get(SESSION_MANAGER_T *manager,
		uint64_t session_id) {
	pthread_mutex_lock(&manager->lock);

	for (uint32_t i = 0; i < MAX_SESSIONS; i++) {
		SESSION_T *session = manager->sessions[i];
		if (session != NULL && session->session_id == session_id) {
			pthread_mutex_unlock(&manager->lock);
			return session;
		}
	}

	pthread_mutex_unlock(&manager->lock);

	return NULL;
}

int session_manager_udp_bind(SESSION_MANAGER_T *manager, uint16_t port) {
	manager->udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (manager->udp_socket < 0) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to create UDP tunnel socket");
		return 1;
	}

	int opt = 1;
	setsockopt(manager->udp_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	if (bind(manager->udp_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to bind UDP tunnel socket");
		close(manager->udp_socket);
		manager->udp_socket = -1;
		return 1;
	}

	return 0;
}

void *session_manager_udp_listener(void *arg) {
	SESSION_MANAGER_T *manager = (SESSION_MANAGER_T *)arg;
	uint8_t *buffer = malloc(65536);
	if (buffer == NULL) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to allocate UDP receive buffer");
		return NULL;
	}

	while (manager->udp_socket >= 0) {
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(manager->udp_socket, &readfds);
		struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};

		int ready = select(manager->udp_socket + 1, &readfds, NULL, NULL, &timeout);

		if (manager->udp_socket < 0)
			break;
		if (ready <= 0)
			continue;

		struct sockaddr_in from = {0};
		socklen_t from_len = sizeof(from);

		ssize_t received = recvfrom(manager->udp_socket, buffer, 65536, 0,
				(struct sockaddr *)&from, &from_len);
		if (received < (ssize_t)sizeof(uint64_t))
			continue;

		uint64_t session_id;
		memcpy(&session_id, buffer, sizeof(session_id));
		session_id = be64toh(session_id);

		SESSION_T *session = session_manager_get(manager, session_id);
		if (session == NULL)
			continue;

		pthread_mutex_lock(&manager->lock);
		session->udp.peer_addr = from;
		session->udp.peer_known = 1;
		pthread_mutex_unlock(&manager->lock);
	}

	free(buffer);
	return NULL;
}

