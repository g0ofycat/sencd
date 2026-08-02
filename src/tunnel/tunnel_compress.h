// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/shared/tunnel_compress.h: zstd compression for all tunnel packets
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#ifndef TUNNEL_COMPRESS_H
#define TUNNEL_COMPRESS_H

#include <endian.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <zstd.h>

//--============
// -- CONSTS
//--============

#define TUNNEL_COMPRESS_FLAG_NONE 0x00
#define TUNNEL_COMPRESS_FLAG_ZSTD 0x01
#define TUNNEL_COMPRESS_HEADER_SIZE 5

//--============
// -- DEFINITIONS
//--============

/// @brief conditionally zstd-compress a raw packet; falls back to storing it raw if compression doesn't actually shrink it
/// @param *raw, raw_len: input packet
/// @param *out_buffer, out_capacity: output buffer
/// @return size_t: bytes written, 0 on failure
size_t tunnel_pipeline_compress(const uint8_t *raw, size_t raw_len,
								uint8_t *out_buffer, size_t out_capacity);

/// @brief reverse of tunnel_pipeline_compress
/// @param *in_buffer, in_len: data produced by tunnel_pipeline_compress
/// @param *out_buffer, out_capacity: output buffer
/// @return size_t: bytes written, 0 on failure
size_t tunnel_pipeline_decompress(const uint8_t *in_buffer, size_t in_len,
								  uint8_t *out_buffer, size_t out_capacity);

#endif
