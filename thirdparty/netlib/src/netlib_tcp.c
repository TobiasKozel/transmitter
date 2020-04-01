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
 */

#include "platform.h"
#include "netlib.h"

/* The network API for TCP sockets */

struct _tcp_socket {
	int ready;
	SOCKET channel;
	ip_address remoteAddress;
	ip_address localAddress;
	int sflag;
};

/* Open a TCP network socket
   If 'remote' is NULL, this creates a local server socket on the given port,
   otherwise a TCP connection to the remote host and port is attempted.
   The newly created socket is returned, or NULL if there was an error.
*/
tcp_socket netlib_tcp_open(ip_address* ip)
{
	tcp_socket sock;
	struct sockaddr_in sock_addr;

	/* Allocate a TCP socket structure */
	sock = (tcp_socket) malloc(sizeof(*sock));
	if (sock == NULL)
	{
		netlib_set_error("Out of memory");
		goto error_return;
	}

	/* Open the socket */
	sock->channel = socket(AF_INET, SOCK_STREAM, 0);
	if (sock->channel == INVALID_SOCKET)
	{
		netlib_set_error("Couldn't create socket");
		goto error_return;
	}

	/* Connect to remote, or bind locally, as appropriate */
	if ((ip->host != INADDR_NONE) && (ip->host != INADDR_ANY))
	{

		// #########  Connecting to remote

		memset(&sock_addr, 0, sizeof(sock_addr));
		sock_addr.sin_family = AF_INET;
		sock_addr.sin_addr.s_addr = ip->host;
		sock_addr.sin_port = ip->port;

		/* Connect to the remote host */
		if (connect(sock->channel, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
		{
			netlib_set_error("Couldn't connect to remote host");
			goto error_return;
		}
		sock->sflag = 0;
	}
	else
	{

		// ##########  Binding locally

		memset(&sock_addr, 0, sizeof(sock_addr));
		sock_addr.sin_family = AF_INET;
		sock_addr.sin_addr.s_addr = INADDR_ANY;
		sock_addr.sin_port = ip->port;

		/*
		 * Windows gets bad mojo with SO_REUSEADDR:
		 * http://www.devolution.com/pipermail/sdl/2005-September/070491.html
		 *   --ryan.
		 */
#ifndef WIN32
		/* allow local address reuse */
		{ 
			int yes = 1;
			setsockopt(sock->channel, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));
		}
#endif

		/* Bind the socket for listening */
		if (bind(sock->channel, (struct sockaddr *)&sock_addr,
			sizeof(sock_addr)) == SOCKET_ERROR)
		{
			netlib_set_error("Couldn't bind to local port");
			goto error_return;
		}

		if (listen(sock->channel, 5) == SOCKET_ERROR)
		{
			netlib_set_error("Couldn't listen to local port");
			goto error_return;
		}

#if defined(WIN32)
		{
			unsigned long mode = 1;
			ioctlsocket(sock->channel, FIONBIO, &mode);
		}
#else
		#warning How do we set non - blocking mode on other operating systems ?
#endif
			sock->sflag = 1;
	}
	sock->ready = 0;

#ifdef TCP_NODELAY
	/* Set the nodelay TCP option for real-time games */
	{
		int yes = 1;
		setsockopt(sock->channel, IPPROTO_TCP, TCP_NODELAY, (char*) &yes, sizeof(yes));
	}
#else
	#warning Building without TCP_NODELAY
#endif /* TCP_NODELAY */

	/* Fill in the channel host address */
	sock->remoteAddress.host = sock_addr.sin_addr.s_addr;
	sock->remoteAddress.port = sock_addr.sin_port;

	/* The socket is ready */
	return sock;

error_return:
	netlib_tcp_close(sock);
	return NULL;
}

/* Accept an incoming connection on the given server socket.
   The newly created socket is returned, or NULL if there was an error.
*/
tcp_socket netlib_tcp_accept(tcp_socket server)
{
	tcp_socket sock;
	struct sockaddr_in sock_addr;
	socklen_t sock_alen;

	/* Only server sockets can accept */
	if (!server->sflag)
	{
		netlib_set_error("Only server sockets can accept()");
		return NULL;
	}
	server->ready = 0;

	/* Allocate a TCP socket structure */
	sock = (tcp_socket) malloc(sizeof(*sock));
	if (sock == NULL)
	{
		netlib_set_error("Out of memory");
		goto error_return;
	}

	/* Accept a new TCP connection on a server socket */
	sock_alen = sizeof(sock_addr);
	sock->channel = accept(server->channel, (struct sockaddr*) &sock_addr, &sock_alen);
	if (sock->channel == INVALID_SOCKET)
	{
		netlib_set_error("accept() failed");
		goto error_return;
	}

#ifdef WIN32
	{
		/* passing a zero value, socket mode set to block on */
		unsigned long mode = 0;
		ioctlsocket(sock->channel, FIONBIO, &mode);
	}
#elif defined(O_NONBLOCK)
	{
		int flags = fcntl(sock->channel, F_GETFL, 0);
		fcntl(sock->channel, F_SETFL, flags & ~O_NONBLOCK);
	}
#endif /* WIN32 */

	sock->remoteAddress.host = sock_addr.sin_addr.s_addr;
	sock->remoteAddress.port = sock_addr.sin_port;

	sock->sflag = 0;
	sock->ready = 0;

	/* The socket is ready */
	return sock;

error_return:
	netlib_tcp_close(sock);
	return NULL;
}

/* Get the IP address of the remote system associated with the socket.
   If the socket is a server socket, this function returns NULL.
*/
ip_address* netlib_tcp_get_peer_address(tcp_socket sock)
{
	if (sock->sflag)
		return NULL;

	return &sock->remoteAddress;
}

/* Send 'len' bytes of 'data' over the non-server socket 'sock'
   This function returns the actual amount of data sent.  If the return value
   is less than the amount of data sent, then either the remote connection was
   closed, or an unknown socket error occurred.
*/
int netlib_tcp_send(tcp_socket sock, const void* datap, int len)
{
	const uint8_t *data = (const uint8_t* )datap;   /* For pointer arithmetic */
	int sent, left;

	/* Server sockets are for accepting connections only */
	if (sock->sflag)
	{
		netlib_set_error("Server sockets cannot send");
		return -1;
	}

	/* Keep sending data until it's sent or an error occurs */
	left = len;
	sent = 0;
	netlib_set_last_error(0);

	do
	{
		len = send(sock->channel, (const char*) data, left, 0);
		if (len > 0)
		{
			sent += len;
			left -= len;
			data += len;
		}
	} while ((left > 0) && ((len > 0) || (netlib_get_last_error() == EINTR)));

	return sent;
}

/* Receive up to 'maxlen' bytes of data over the non-server socket 'sock',
   and store them in the data pointed to by 'data'.
   This function returns the actual amount of data received.  If the return
   value is less than or equal to zero, then either the remote connection was
   closed, or an unknown socket error occurred.
*/
int netlib_tcp_recv(tcp_socket sock, void* data, int maxlen)
{
	int len;

	/* Server sockets are for accepting connections only */
	if (sock->sflag)
	{
		netlib_set_error("Server sockets cannot receive");
		return -1;
	}

	netlib_set_last_error(0);
	do
	{
		len = recv(sock->channel, (char*) data, maxlen, 0);
	} while (netlib_get_last_error() == EINTR);

	sock->ready = 0;
	return len;
}

/* Close a TCP network socket */
void netlib_tcp_close(tcp_socket sock)
{
	if (sock != NULL)
	{
		if (sock->channel != INVALID_SOCKET)
			closesocket(sock->channel);
		
		free(sock);
	}
}

/* === Buffer API (Not part of SDL_net) === */

int netlib_tcp_send_buf(tcp_socket sock, netlib_byte_buf* buf)
{
	if (!buf)
		return -1;
	return netlib_tcp_send(sock, buf->data, buf->length);
}

int netlib_tcp_send_buf_smart(tcp_socket sock, netlib_byte_buf* buf)
{
    if (!buf)
        return -1;
    return netlib_tcp_send(sock, buf->data, buf->write_pos);
}

int netlib_tcp_recv_buf(tcp_socket sock, netlib_byte_buf* buf)
{
	if (!buf)
		return -1;
	return netlib_tcp_recv(sock, buf->data, buf->length);
}
