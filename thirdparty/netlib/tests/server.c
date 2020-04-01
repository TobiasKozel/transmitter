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
 * Source: libsdl.org/projects/SDL_net/docs/demos/tcpserver.c
 */

#include <stdlib.h>
#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#define Sleep sleep
#endif
#ifdef WINDOWS
#include <Windows.h>
#endif
#include <netlib.h>

int main(int argc, char **argv)
{
	ip_address ip, *remoteip;
	tcp_socket server, client;
	char message[1024];
	int len;
	uint32_t server_ip;
	uint32_t ipaddr;
	uint16_t port;

	/* check our commandline */
	if (argc<2)
	{
		printf("usage:\n [port]\n", argv[0]);
		exit(0);
	}

	/* initialize netlib */
	if (netlib_init() == -1)
	{
		printf("netlib_init: %s\n", netlib_get_error());
		exit(2);
	}

	/* get the port from the commandline */
	port = (uint16_t)strtol(argv[1], NULL, 0);

	/* Resolve the argument into an IPaddress type */
	if (netlib_resolve_host(&ip, NULL, port) == -1)
	{
		printf("netlib_resolve_host: %s\n", netlib_get_error());
		exit(3);
	}
	server_ip = ip.host;
	printf("Started server on %d.%d.%d.%d port %hu\n",
		server_ip >> 24, server_ip >> 16 & 0xff,
		server_ip >> 8 & 0xff, server_ip & 0xff, port);

	/* open the server socket */
	server = netlib_tcp_open(&ip);
	if (!server)
	{
		printf("netlib_tcp_open: %s\n", netlib_get_error());
		exit(4);
	}

	while (1)
	{
		/* try to accept a connection */
		client = netlib_tcp_accept(server);
		if (!client)
		{
			Sleep(100);
			continue;
		}

		/* get the clients IP and port number */
		remoteip = netlib_tcp_get_peer_address(client);
		if (!remoteip)
		{
			printf("netlib_tcp_get_peer_address: %s\n", netlib_get_error());
			continue;
		}

		/* print out the clients IP and port number */
		ipaddr = netlib_swap_BE32(remoteip->host);
		printf("Accepted a connection from %d.%d.%d.%d port %hu\n",
			ipaddr >> 24,
			(ipaddr >> 16) & 0xff,
			(ipaddr >> 8) & 0xff,
			ipaddr & 0xff,
			remoteip->port);

		/* read the data from client */
		netlib_byte_buf* buf = netlib_alloc_byte_buf(20);

		if (!buf)
		{
			printf("netlib_alloc_byte_buf: %s\n", netlib_get_error());
			netlib_tcp_close(client);
			break;
		}

		if (netlib_tcp_recv_buf(client, buf) < buf->length)
		{
			printf("netlib_tcp_close: %s\n", netlib_get_error());
			netlib_tcp_close(client);
			continue;
		}

		/* Recieve data over byte buffer */
		uint32_t a, b;
		int32_t c, d;
		float f = 0.f;

		netlib_read_uint32(buf, &a);
		netlib_read_uint32(buf, &b);
		netlib_read_int32(buf, &c);
		netlib_read_int32(buf, &d);
		netlib_read_float(buf, &f);

		printf("Received: %u %u %i %i %f\n", a, b, c, d, f);
		netlib_free_byte_buf(buf);

		if (message[0] == 'Q')
		{
			printf("Quitting on a Q received\n");
			break;
		}
	}

	/* shutdown netlib */
	netlib_quit();

	return 0;
}