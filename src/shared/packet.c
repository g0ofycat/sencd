// TODO: convert packet headers to network and from network byte order

#include "packet.h"

//--============
// -- FORWARD DECL.
//--============

/// @brief send all data from buffer to socket
/// @param socket
/// @param *buffer
/// @param length
/// @return int: success bool
static int send_all(int socket, const void *buffer, size_t length);

/// @brief recv all data from socket to buffer
/// @param socket
/// @param *buffer
/// @param length
/// @return int: success bool
static int recv_all(int socket, void *buffer, size_t length);

//--============
// -- LOGIC
//--============

void packet_init(PACKET *packet) {
	memset(packet, 0, sizeof(*packet));
}

void packet_destroy(PACKET *packet) {
	free(packet->payload);
	memset(packet, 0, sizeof(*packet));
}

int packet_send(int socket, PACKET *packet) {
	PACKET_HEADER header = packet->header;

	if (header.payload_length > MAX_PACKET_SIZE) {
		log_msg(ERROR_MSG, OTHER_RT, "Packet exceeds maximum size, aborting...");
		return 1;
	}

	if (send_all(
				socket,
				&header,
				sizeof(PACKET_HEADER)) < 0)
	{
		log_msg(ERROR_MSG, OTHER_RT, "Failed to send packet, aborting...");
		return 1;
	}

	if (header.payload_length > 0) {
		if (send_all(
					socket,
					packet->payload,
					header.payload_length) < 0) {
			log_msg(ERROR_MSG, OTHER_RT, "Failed to send packet payload, aborting...");
			return 1;
		}
	}

	return 0;
}

int packet_receive(int socket, PACKET *packet) {
	PACKET_HEADER header;

	packet_destroy(packet);

	if (recv_all(
				socket,
				&header,
				sizeof(PACKET_HEADER)) < 0)
	{
		log_msg(ERROR_MSG, OTHER_RT, "Failed to receive packet, aborting...");
		return 1;
	}

	packet->header = header;

	if (header.payload_length > MAX_PACKET_SIZE) {
		log_msg(ERROR_MSG, OTHER_RT, "Packet exceeds maximum size, aborting...");
		return 1;
	}

	if (header.payload_length > 0)
	{
		packet->payload = malloc(header.payload_length);

		if (packet->payload == NULL)
		{
			log_msg(ERROR_MSG, OTHER_RT, "Payload is NULL");
			return 1;
		}

		if (recv_all(
					socket,
					packet->payload,
					header.payload_length) < 0)
		{
			log_msg(ERROR_MSG, OTHER_RT, "Failed to receive packet payload, aborting...");
			packet_destroy(packet);
			return 1;
		}
	}

	return 0;
}

PACKET packet_construct(PACKET_CONSTRUCTOR_T interface) {
	PACKET packet;

	packet_init(&packet);

	packet.header.type = interface.header_type;
	packet.header.version = interface.header_version;

	packet.header.payload_length = strlen(interface.message);
	packet.payload = malloc(packet.header.payload_length);

	if (packet.payload == NULL)
	{
		log_msg(WARN_MSG, OTHER_RT, "Failed to allocate packet, returning...");
		packet.header.payload_length = 0;
		return packet;
	}

	memcpy(
			packet.payload,
			interface.message,
			packet.header.payload_length
		  );

	return packet;
}

//--============
// -- INTERNAL
//--============

static int send_all(int socket, const void *buffer, size_t length) {
	size_t total_sent = 0;
	while (total_sent < length) {
		ssize_t sent = send(
				socket,
				(char *)buffer + total_sent,
				length - total_sent,
				0
				);

		if (sent <= 0) {
			return -1;
		}

		total_sent += sent;
	}

	return 0;
}

static int recv_all(int socket, void *buffer, size_t length) {
	size_t total_received = 0;
	while (total_received < length) {
		ssize_t received = recv(
				socket,
				(char *)buffer + total_received,
				length - total_received,
				0
				);

		if (received <= 0) {
			return -1;
		}

		total_received += received;
	}

	return 0;
}
