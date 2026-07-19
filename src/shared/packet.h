#ifndef PACKET_H
#define PACKET_H

#include <stdint.h>

//--============
// -- TYPEDEFS
//--============

typedef enum {
	PACKET_SERVER_HELLO,
	PACKET_CLIENT_HELLO,

	PACKET_AUTH_REQUEST,
	PACKET_AUTH_SUCCESS,
	PACKET_AUTH_FAIL,

	PACKET_PING,
	PACKET_PONG,

	PACKET_TUNNEL_DATA,

	PACKET_DISCONNECT,

	PACKET_ERROR
} PACKET_T;

typedef struct {
	uint32_t payload_length;
	uint16_t flags;
	uint8_t  type;
	uint8_t  version;
} PACKET_HEADERS;

#endif
