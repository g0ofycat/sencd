#include "tunnel_packet.h"

#include <string.h>
#include <endian.h>

//--============
// -- LOGIC
//--============

int tunnel_packet_encode(const TUNNEL_PACKET_ENCODE_T *in, CRYPTO_CONTEXT_T *ctx,
		unsigned char *out_buffer, size_t *out_length) {
	if (in->plaintext_len > TUNNEL_PACKET_MAX_PLAINTEXT)
		return 1;

	uint64_t be_session_id = htobe64(in->session_id);
	uint64_t be_counter = htobe64(in->counter);

	memcpy(out_buffer, &be_session_id, sizeof(be_session_id));
	memcpy(out_buffer + sizeof(be_session_id), &be_counter, sizeof(be_counter));

	unsigned long long ciphertext_len = 0;
	if (crypto_encrypt_packet(ctx, in->counter, in->plaintext, in->plaintext_len,
				out_buffer + TUNNEL_HEADER_SIZE, &ciphertext_len) != 0)
		return 1;

	*out_length = TUNNEL_HEADER_SIZE + (size_t)ciphertext_len;
	return 0;
}

int tunnel_packet_peek_session_id(const unsigned char *buffer, size_t buffer_len, uint64_t *session_id_out) {
	if (buffer_len < TUNNEL_HEADER_SIZE)
		return 1;

	uint64_t be_session_id;
	memcpy(&be_session_id, buffer, sizeof(be_session_id));
	*session_id_out = be64toh(be_session_id);
	return 0;
}

int tunnel_packet_decode(const unsigned char *buffer, size_t buffer_len, CRYPTO_CONTEXT_T *ctx,
		unsigned char *out_plaintext, unsigned long long *out_length) {
	if (buffer_len < TUNNEL_HEADER_SIZE + TUNNEL_AEAD_TAG_SIZE ||
			buffer_len > TUNNEL_PACKET_MAX_SIZE)
		return 1;

	uint64_t be_counter;
	memcpy(&be_counter, buffer + sizeof(uint64_t), sizeof(be_counter));
	uint64_t counter = be64toh(be_counter);

	const unsigned char *ciphertext = buffer + TUNNEL_HEADER_SIZE;
	size_t ciphertext_len = buffer_len - TUNNEL_HEADER_SIZE;

	if (crypto_decrypt_packet(ctx, counter, ciphertext, ciphertext_len,
				out_plaintext, out_length) != 0)
		return 1;

	return 0;
}
