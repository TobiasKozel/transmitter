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

struct netlib_socket
{
	int ready;
	SOCKET channel;
};

struct _netlib_socket_set
{
	int numsockets;
	int maxsockets;
	struct netlib_socket** sockets;
};

/* Allocate a socket set for use with SDLNet_CheckSockets()
   This returns a socket set for up to 'maxsockets' sockets, or NULL if
   the function ran out of memory.
*/
netlib_socket_set netlib_alloc_socket_set(int maxsockets)
{
	struct _netlib_socket_set* set;
	int i;

	set = (struct _netlib_socket_set*) malloc(sizeof(*set));
	if (set != NULL)
	{
		set->numsockets = 0;
		set->maxsockets = maxsockets;
		set->sockets = (struct netlib_socket**) malloc(maxsockets * sizeof(*set->sockets));
		
		if (set->sockets != NULL)
		{
			for (i = 0; i < maxsockets; ++i)
				set->sockets[i] = NULL;
		}
		else
		{
			free(set);
			set = NULL;
		}
	}

	return set;
}

/* Add a socket to a set of sockets to be checked for available data */
int netlib_add_socket(netlib_socket_set set, netlib_generic_socket sock)
{
	if (sock != NULL)
	{
		if (set->numsockets == set->maxsockets)
		{
			netlib_set_error("socketset is full");
			return(-1);
		}
		set->sockets[set->numsockets++] = (struct netlib_socket*) sock;
	}

	return set->numsockets;
}

/* Remove a socket from a set of sockets to be checked for available data */
int netlib_del_socket(netlib_socket_set set, netlib_generic_socket sock)
{
	int i;

	if (sock != NULL)
	{
		for (i = 0; i<set->numsockets; ++i)
		{
			if (set->sockets[i] == (struct netlib_socket*) sock)
				break;
		}
		
		if (i == set->numsockets)
		{
			netlib_set_error("socket not found in socketset");
			return -1;
		}
		
		--set->numsockets;
		
		for (; i<set->numsockets; ++i)
		{
			set->sockets[i] = set->sockets[i + 1];
		}
	}

	return(set->numsockets);
}

/* This function checks to see if data is available for reading on the
   given set of sockets.  If 'timeout' is 0, it performs a quick poll,
   otherwise the function returns when either data is available for
   reading, or the timeout in milliseconds has elapsed, which ever occurs
   first.  This function returns the number of sockets ready for reading,
   or -1 if there was an error with the select() system call.
*/
int netlib_check_socket_set(netlib_socket_set set, uint32_t timeout)
{
	int i;
	SOCKET maxfd;
	int retval;
	struct timeval tv;
	fd_set mask;

	/* Find the largest file descriptor */
	maxfd = 0;
	for (i = set->numsockets - 1; i >= 0; --i)
	{
		if (set->sockets[i]->channel > maxfd)
			maxfd = set->sockets[i]->channel;
	}

	/* Check the file descriptors for available data */
	do 
	{
		netlib_set_last_error(0);

		/* Set up the mask of file descriptors */
		FD_ZERO(&mask);
		for (i = set->numsockets - 1; i >= 0; --i)
			FD_SET(set->sockets[i]->channel, &mask);

		/* Set up the timeout */
		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;

		/* Look! */
		retval = select(maxfd + 1, &mask, NULL, NULL, &tv);
	} while (netlib_get_last_error() == EINTR);

	/* Mark all file descriptors ready that have data available */
	if (retval > 0)
	{
		for (i = set->numsockets - 1; i >= 0; --i)
		{
			if (FD_ISSET(set->sockets[i]->channel, &mask))
				set->sockets[i]->ready = 1;
		}
	}
	return(retval);
}

/* Free a set of sockets allocated by SDL_NetAllocSocketSet() */
extern void netlib_free_socket_set(netlib_socket_set set)
{
	if (set)
	{
		free(set->sockets);
		free(set);
	}
}