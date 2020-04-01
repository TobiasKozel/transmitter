/**
 * This file is part of netlib
 * which is licensed under the zlib license
 *
 * github.com/univrsal/netlib
 * netlib is a small network library
 * heavily based on SDL_net without
 * the dependency on SDL
 *
 * Documentation is directly taken from SDL_net
 * I take no credit for this code, I only
 * reformatted it for my needs and made slight changes
 * Source: libsdl.org/projects/SDL_net/docs/demos/tcpclient.c
 */

#include <stdio.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <string.h>
#include <netlib.h>

#define MAX_ADDRESSES   10

void show_interfaces()
{
	ip_address addresses[MAX_ADDRESSES];
	int i, count;

	count = netlib_get_local_addresses(addresses, MAX_ADDRESSES);
	printf("Found %d local addresses\n", count);
	for (i = 0; i < count; ++i) {
		printf("%d: %d.%d.%d.%d - %s\n", i + 1,
			(addresses[i].host >> 0) & 0xFF,
			(addresses[i].host >> 8) & 0xFF,
			(addresses[i].host >> 16) & 0xFF,
			(addresses[i].host >> 24) & 0xFF,
			netlib_resolve_ip(&addresses[i]));
	}
}

int main(int argc, char **argv)
{
	show_interfaces();

	ip_address ip;
	tcp_socket sock;
	char message[1024];
	int len;
	uint16_t port;

	/* check our commandline */
	if (argc<3)
	{
		printf("usage:\n [host] [port]\n");
		exit(0);
	}

	/* initialize netlib */
	if (netlib_init() == -1)
	{
		printf("netlib_init: %s\n", netlib_get_error());
		exit(2);
	}

	/* get the port from the commandline */
	port = (uint16_t)strtol(argv[2], NULL, 0);

	/* Resolve the argument into an IPaddress type */
	if (netlib_resolve_host(&ip, argv[1], port) == -1)
	{
		printf("netlib_resolve_host: %s\n", netlib_get_error());
		exit(3);
	}

	/* open the server socket */
	sock = netlib_tcp_open(&ip);
	if (!sock)
	{
		printf("netlib_tcp_open: %s\n", netlib_get_error());
		exit(4);
	}

	netlib_byte_buf* buf = netlib_alloc_byte_buf(20);

	netlib_write_uint32(buf, UINT32_MAX);
	netlib_write_uint32(buf, 0);
	netlib_write_int32(buf, INT32_MAX);
	netlib_write_int32(buf, INT32_MIN);
	netlib_write_float(buf, 1.337f);

	if (netlib_tcp_send_buf(sock, buf) < buf->length)
	{
		printf("netlib_tcp_send: %s\n", netlib_get_error());
	}

	netlib_free_byte_buf(buf);

	netlib_tcp_close(sock);

	/* shutdown netlib */
	netlib_quit();

	return 0;
}