// SPDX-License-Identifier: GPL-3.0-only
/*
 *	src/platform/linux/tun.c: Linux TUN device implementation
 *
 *	Copyright (C) 2026 Clinton Ung-davy
 */

#include "../../tunnel/tun.h"

#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//--============
// -- LOGIC
//--============

struct TUN_DEVICE {
	char name[IFNAMSIZ];
	int fd;
};

static int tun_assign_address(const char *ifname, const TUN_CONFIG_T *config) {
	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to create control socket for TUN config");
		return 1;
	}

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);

	struct sockaddr_in addr = {0};
	addr.sin_family = AF_INET;
	if (inet_pton(AF_INET, config->address, &addr.sin_addr) != 1) {
		log_msg(ERROR_MSG, OTHER_RT, "Invalid TUN address");
		close(sock);
		return 1;
	}

	memcpy(&ifr.ifr_addr, &addr, sizeof(addr));
	if (ioctl(sock, SIOCSIFADDR, &ifr) < 0) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to assign TUN address");
		close(sock);
		return 1;
	}

	struct sockaddr_in mask = {0};
	mask.sin_family = AF_INET;
	if (inet_pton(AF_INET, config->netmask, &mask.sin_addr) != 1) {
		log_msg(ERROR_MSG, OTHER_RT, "Invalid TUN netmask");
		close(sock);
		return 1;
	}

	memcpy(&ifr.ifr_netmask, &mask, sizeof(mask));
	if (ioctl(sock, SIOCSIFNETMASK, &ifr) < 0) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to assign TUN netmask");
		close(sock);
		return 1;
	}

	if (ioctl(sock, SIOCGIFFLAGS, &ifr) < 0) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to read TUN interface flags");
		close(sock);
		return 1;
	}

	ifr.ifr_flags |= (IFF_UP | IFF_RUNNING);
	if (ioctl(sock, SIOCSIFFLAGS, &ifr) < 0) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to bring TUN interface up");
		close(sock);
		return 1;
	}

	close(sock);
	return 0;
}

TUN_DEVICE_T *tun_open(const TUN_CONFIG_T *config) {
	int fd = open("/dev/net/tun", O_RDWR);
	if (fd < 0) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to open /dev/net/tun (are you root / do you have CAP_NET_ADMIN?)");
		return NULL;
	}

	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	strncpy(ifr.ifr_name, config->ifname, IFNAMSIZ - 1);

	if (ioctl(fd, TUNSETIFF, &ifr) < 0) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to create TUN interface");
		close(fd);
		return NULL;
	}

	if (tun_assign_address(ifr.ifr_name, config) != 0) {
		close(fd);
		return NULL;
	}

	TUN_DEVICE_T *dev = malloc(sizeof(*dev));
	if (dev == NULL) {
		log_msg(ERROR_MSG, OTHER_RT, "Failed to allocate TUN device struct");
		close(fd);
		return NULL;
	}

	dev->fd = fd;
	strncpy(dev->name, ifr.ifr_name, IFNAMSIZ - 1);

	log_msg(INFO_MSG, OTHER_RT, "TUN interface %s up (%s)", dev->name, config->address);

	return dev;
}

ssize_t tun_read(TUN_DEVICE_T *dev, uint8_t *buffer, size_t buffer_size) {
	log_msg(INFO_MSG, OTHER_RT, "Read packet from tunnel %s", dev);
	return read(dev->fd, buffer, buffer_size);
}

ssize_t tun_write(TUN_DEVICE_T *dev, const uint8_t *buffer, size_t length) {
	log_msg(INFO_MSG, OTHER_RT, "Wrote packet to tunnel %s", dev);
	return write(dev->fd, buffer, length);
}

int tun_get_fd(TUN_DEVICE_T *dev) {
	return dev->fd;
}

void tun_close(TUN_DEVICE_T *dev) {
	if (dev == NULL)
		return;

	if (dev->fd >= 0)
		close(dev->fd);

	free(dev);
}
