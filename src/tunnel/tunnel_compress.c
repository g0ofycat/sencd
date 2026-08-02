#include "tunnel_compress.h"

//--============
// -- LOGIC
//--============

size_t tunnel_pipeline_compress(const uint8_t *raw, size_t raw_len,
		uint8_t *out_buffer, size_t out_capacity) {
	if (out_capacity < TUNNEL_COMPRESS_HEADER_SIZE)
		return 0;

	size_t max_payload = out_capacity - TUNNEL_COMPRESS_HEADER_SIZE;
	size_t compressed_len = 0;

	if (ZSTD_compressBound(raw_len) <= max_payload) {
		compressed_len = ZSTD_compress(out_buffer + TUNNEL_COMPRESS_HEADER_SIZE,
				max_payload, raw, raw_len, 3);
		if (ZSTD_isError(compressed_len))
			compressed_len = 0;
	}

	if (compressed_len > 0 && compressed_len < raw_len) {
		out_buffer[0] = TUNNEL_COMPRESS_FLAG_ZSTD;
		uint32_t be_len = htobe32((uint32_t)raw_len);
		memcpy(out_buffer + 1, &be_len, sizeof(be_len));
		return TUNNEL_COMPRESS_HEADER_SIZE + compressed_len;
	}

	if (raw_len > max_payload)
		return 0;

	out_buffer[0] = TUNNEL_COMPRESS_FLAG_NONE;
	memset(out_buffer + 1, 0, 4);
	memcpy(out_buffer + TUNNEL_COMPRESS_HEADER_SIZE, raw, raw_len);
	return TUNNEL_COMPRESS_HEADER_SIZE + raw_len;
}

size_t tunnel_pipeline_decompress(const uint8_t *in_buffer, size_t in_len,
		uint8_t *out_buffer, size_t out_capacity) {
	if (in_len < TUNNEL_COMPRESS_HEADER_SIZE)
		return 0;

	uint8_t flag = in_buffer[0];
	const uint8_t *payload = in_buffer + TUNNEL_COMPRESS_HEADER_SIZE;
	size_t payload_len = in_len - TUNNEL_COMPRESS_HEADER_SIZE;

	if (flag == TUNNEL_COMPRESS_FLAG_NONE) {
		if (payload_len > out_capacity)
			return 0;
		memcpy(out_buffer, payload, payload_len);
		return payload_len;
	}

	if (flag == TUNNEL_COMPRESS_FLAG_ZSTD) {
		uint32_t be_len;
		memcpy(&be_len, in_buffer + 1, sizeof(be_len));
		uint32_t original_len = be32toh(be_len);
		if (original_len > out_capacity)
			return 0;

		size_t result = ZSTD_decompress(out_buffer, out_capacity, payload, payload_len);
		if (ZSTD_isError(result) || result != original_len)
			return 0;
		return result;
	}

	return 0;
}
