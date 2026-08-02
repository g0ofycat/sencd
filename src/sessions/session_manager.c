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
			crypto_identity_load_or_create(&manager->identity, path) != 0)
		log_msg(ERROR_MSG, SERVER_RT, "Failed to load or create server identity");

	TUN_CONFIG_T tun_config = {.address = SERVER_DEFAULT_VNI, .netmask = DEFAULT_NETMASK};
	manager->tun = tun_open(&tun_config);
	if (manager->tun == NULL)
		log_msg(ERROR_MSG, SERVER_RT, "Failed to open TUN device");
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

	tun_close(manager->tun);
	manager->tun = NULL;
}

int session_manager_add(SESSION_MANAGER_T *manager, SESSION_T *session) {
	pthread_mutex_lock(&manager->lock);

	for (uint32_t i = 0; i < MAX_SESSIONS; i++) {
		if (manager->sessions[i] == NULL) {
			session->tunnel_ip_octet = (uint8_t)(i + 2);
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
	uint8_t *buffer = malloc(TUNNEL_PACKET_MAX_SIZE);
	uint8_t *plaintext = malloc(TUNNEL_PACKET_MAX_SIZE);
	if (buffer == NULL || plaintext == NULL) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to allocate UDP receive buffers");
		free(buffer);
		free(plaintext);
		return NULL;
	}

	SESSION_MANAGER_T *manager = (SESSION_MANAGER_T *)arg;

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

		ssize_t received = recvfrom(manager->udp_socket, buffer, TUNNEL_PACKET_MAX_SIZE, 0,
				(struct sockaddr *)&from, &from_len);
		if (received <= 0)
			continue;

		uint64_t session_id;
		if (tunnel_packet_peek_session_id(buffer, (size_t)received, &session_id) != 0)
			continue;

		SESSION_T *session = session_manager_get(manager, session_id);
		if (session == NULL)
			continue;

		pthread_mutex_lock(&manager->lock);
		session->udp.peer_addr = from;
		session->udp.peer_known = 1;
		pthread_mutex_unlock(&manager->lock);

		unsigned long long plaintext_len = 0;
		if (tunnel_packet_decode(buffer, (size_t)received, &session->crypto,
					plaintext, &plaintext_len) != 0)
			continue;

		if (manager->tun != NULL) {
			tun_write(manager->tun, plaintext, (size_t)plaintext_len);
		}
	}

	free(buffer);
	free(plaintext);
	return NULL;
}

void *session_manager_tun_sender(void *arg) {
	SESSION_MANAGER_T *manager = (SESSION_MANAGER_T *)arg;
	if (manager->tun == NULL) {
		log_msg(ERROR_MSG, SERVER_RT, "TUN device unavailable, tunnel sender thread exiting");
		return NULL;
	}

	uint8_t *raw = malloc(TUNNEL_PACKET_MAX_PLAINTEXT);
	uint8_t *encoded = malloc(TUNNEL_PACKET_MAX_SIZE);
	if (raw == NULL || encoded == NULL) {
		log_msg(ERROR_MSG, SERVER_RT, "Failed to allocate TUN sender buffers");
		free(raw);
		free(encoded);
		return NULL;
	}

	int tun_fd = tun_get_fd(manager->tun);

	while (manager->udp_socket >= 0) {
		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(tun_fd, &readfds);
		struct timeval timeout = {.tv_sec = 1, .tv_usec = 0};

		int ready = select(tun_fd + 1, &readfds, NULL, NULL, &timeout);

		if (manager->udp_socket < 0)
			break;
		if (ready <= 0)
			continue;

		ssize_t received = tun_read(manager->tun, raw, TUNNEL_PACKET_MAX_PLAINTEXT);
		if (received < 20)
			continue;

		uint8_t dest_octet = raw[19];

		pthread_mutex_lock(&manager->lock);
		SESSION_T *target = NULL;
		for (uint32_t i = 0; i < MAX_SESSIONS; i++) {
			SESSION_T *s = manager->sessions[i];
			if (s != NULL && s->state == SESSION_ESTABLISHED && s->udp.peer_known &&
					s->tunnel_ip_octet == dest_octet) {
				target = s;
				break;
			}
		}

		if (target == NULL) {
			pthread_mutex_unlock(&manager->lock);
			continue;
		}

		struct sockaddr_in peer = target->udp.peer_addr;
		size_t encoded_len = 0;
		int encode_result = tunnel_packet_encode(&(TUNNEL_PACKET_ENCODE_T){
				.session_id = target->session_id,
				.counter = target->crypto.tx_nonce++,
				.plaintext = raw,
				.plaintext_len = (size_t)received
				}, &target->crypto, encoded, &encoded_len);
		pthread_mutex_unlock(&manager->lock);

		if (encode_result != 0)
			continue;

		sendto(manager->udp_socket, encoded, encoded_len, 0,
				(struct sockaddr *)&peer, sizeof(peer));
	}

	free(raw);
	free(encoded);
	return NULL;
}
