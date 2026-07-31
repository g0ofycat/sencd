#include "udp_tunnel.h"

//--============
// -- LOGIC
//--============

void udp_tunnel_init(UDP_TUNNEL_T *tunnel, uint64_t session_id) {
	memset(tunnel, 0, sizeof(*tunnel));
	tunnel->socket = -1;
	tunnel->session_id = session_id;
}

int udp_tunnel_client_open(UDP_TUNNEL_T *tunnel, const char *server_ip, uint16_t port) {
	tunnel->socket = socket(AF_INET, SOCK_DGRAM, 0);
	if (tunnel->socket < 0) {
		log_msg(ERROR_MSG, CLIENT_RT, "Failed to create UDP tunnel socket");
		return 1;
	}

	struct sockaddr_in server = {0};
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (inet_pton(AF_INET, server_ip, &server.sin_addr) != 1) {
		log_msg(ERROR_MSG, CLIENT_RT, "Invalid server IP for UDP tunnel");
		close(tunnel->socket);
		tunnel->socket = -1;
		return 1;
	}

	if (connect(tunnel->socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
		log_msg(ERROR_MSG, CLIENT_RT, "Failed to connect UDP tunnel socket");
		close(tunnel->socket);
		tunnel->socket = -1;
		return 1;
	}

	tunnel->peer_addr = server;
	tunnel->peer_known = 1;

	return 0;
}

void udp_tunnel_close(UDP_TUNNEL_T *tunnel) {
	if (tunnel->socket >= 0)
		close(tunnel->socket);

	tunnel->socket = -1;
	tunnel->peer_known = 0;
}
