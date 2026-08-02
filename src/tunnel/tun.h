// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/tunnel/tun.h: TUN device interface, each OS handles tunneling
 * differently so implementations are in 'src/platform/'
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#ifndef TUN_H
#define TUN_H

#include <sys/types.h>

#include <stddef.h>
#include <stdint.h>

#include "../debug/logs.h"

//--============
// -- CONSTS
//--============

#define SERVER_TUN_INTERFACE_NAME "sencd0"
#define CLIENT_TUN_INTERFACE_NAME "sencd1"

//--============
// -- TYPEDEFS
//--============

typedef struct TUN_DEVICE TUN_DEVICE_T;

typedef struct {
	const char *address;
	const char *netmask;
	const char *ifname;
} TUN_CONFIG_T;

//--============
// -- DEFINITIONS
//--============

/// @brief open and configure a TUN interface
/// @param *config: address/netmask to assign to the interface
/// @return TUN_DEVICE_T*: NULL on failure
TUN_DEVICE_T *tun_open(const TUN_CONFIG_T *config);

/// @brief read one raw IP packet from the TUN device
/// @param *dev
/// @param *buffer, buffer_size: output buffer
/// @return ssize_t: bytes read, -1 on error
ssize_t tun_read(TUN_DEVICE_T *dev, uint8_t *buffer, size_t buffer_size);

/// @brief write one raw IP packet to the TUN device
/// @param *dev
/// @param *buffer, length: packet to write
/// @return ssize_t: bytes written, -1 on error
ssize_t tun_write(TUN_DEVICE_T *dev, const uint8_t *buffer, size_t length);

/// @brief dev->fd
/// @param *dev
/// @return int
int tun_get_fd(TUN_DEVICE_T *dev);

/// @brief close and clean up the TUN device
/// @param *dev
void tun_close(TUN_DEVICE_T *dev);

#endif
