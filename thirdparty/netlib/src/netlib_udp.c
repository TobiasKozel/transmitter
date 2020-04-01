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

#ifdef _WIN32
#define srandom srand
#define random  rand
#endif

struct udp_channel {
	int numbound;
	ip_address address[NETLIB_MAX_UDPADDRESSES];
};

struct _udp_socket {
	int ready;
	SOCKET channel;
	ip_address address;

	struct udp_channel binding[NETLIB_MAX_UDPCHANNELS];

	/* For debugging purposes */
	int packetloss;
};

/* Allocate/free a single UDP packet 'size' bytes long.
The new packet is returned, or NULL if the function ran out of memory.
*/
extern udp_packet* netlib_alloc_packet(int size)
{
	udp_packet* packet;
	int error;


	error = 1;
	packet = (udp_packet*) malloc(sizeof(*packet));

	if (packet != NULL)
	{
		packet->maxlen = size;
		packet->data = (uint8_t*) malloc(size);
		if (packet->data != NULL)
			error = 0;
	}

	if (error)
	{
		netlib_set_error("Out of memory");
		netlib_free_packet(packet);
		packet = NULL;
	}
	return packet;
}

int netlib_resize_packet(udp_packet* packet, int newsize)
{
	uint8_t* newdata;

	newdata = (uint8_t*) malloc(newsize);
	if (newdata != NULL)
	{
		free(packet->data);
		packet->data = newdata;
		packet->maxlen = newsize;
	}

	return packet->maxlen;
}

extern void netlib_free_packet(udp_packet* packet)
{
	if (packet)
	{
		free(packet->data);
		free(packet);
	}
}

/* Allocate/Free a UDP packet vector (array of packets) of 'howmany' packets,
   each 'size' bytes long.
   A pointer to the packet array is returned, or NULL if the function ran out
   of memory.
*/
udp_packet** netlib_alloc_packets(int howmany, int size)
{
	udp_packet **packetV;

	packetV = (udp_packet**) malloc((howmany + 1) * sizeof(*packetV));
	if (packetV != NULL)
	{
		int i;
		for (i = 0; i < howmany; ++i)
		{
			packetV[i] = netlib_alloc_packet(size);
			if (packetV[i] == NULL)
				break;
			
		}
		
		packetV[i] = NULL;

		if (i != howmany)
		{
			netlib_set_error("Out of memory");
			netlib_free_packets(packetV);
			packetV = NULL;
		}
	}

	return packetV;
}

void netlib_free_packets(udp_packet** packetV)
{
	if (packetV)
	{
		int i;
		for (i = 0; packetV[i]; ++i)
			netlib_free_packet(packetV[i]);
		
		free(packetV);
	}
}

/* Open a UDP network socket
   If 'port' is non-zero, the UDP socket is bound to a fixed local port.
*/
udp_socket netlib_udp_open(uint16_t port)
{
	udp_socket sock;
	struct sockaddr_in sock_addr;
	socklen_t sock_len;

	/* Allocate a UDP socket structure */
	sock = (udp_socket) malloc(sizeof(*sock));

	if (sock == NULL)
	{
		netlib_set_error("Out of memory");
		goto error_return;
	}

	memset(sock, 0, sizeof(*sock));
	memset(&sock_addr, 0, sizeof(sock_addr));

	/* Open the socket */
	sock->channel = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock->channel == INVALID_SOCKET)
	{
		netlib_set_error("Couldn't create socket");
		goto error_return;
	}

	/* Bind locally, if appropriate */
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = INADDR_ANY;
	sock_addr.sin_port = netlib_read16(&port);

	/* Bind the socket for listening */
	if (bind(sock->channel, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
	{
		netlib_set_error("Couldn't bind to local port");
		goto error_return;
	}

	/* Get the bound address and port */
	sock_len = sizeof(sock_addr);
	if (getsockname(sock->channel, (struct sockaddr*) &sock_addr, &sock_len) < 0)
	{
		netlib_set_error("Couldn't get socket address");
		goto error_return;
	}

	/* Fill in the channel host address */
	sock->address.host = sock_addr.sin_addr.s_addr;
	sock->address.port = sock_addr.sin_port;

#ifdef SO_BROADCAST
	/* Allow LAN broadcasts with the socket */
	{
		int yes = 1;
		setsockopt(sock->channel, SOL_SOCKET, SO_BROADCAST, (char*) &yes, sizeof(yes));
	}
#endif

#ifdef IP_ADD_MEMBERSHIP
	/* Receive LAN multicast packets on 224.0.0.1
	   This automatically works on Mac OS X, Linux and BSD, but needs
	   this code on Windows.
	*/

	/* A good description of multicast can be found here:
	   http://www.docs.hp.com/en/B2355-90136/ch05s05.html
	*/

	/* FIXME: Add support for joining arbitrary groups to the API */
	{
		struct ip_mreq  g;

		g.imr_multiaddr.s_addr = inet_addr("224.0.0.1");
		g.imr_interface.s_addr = INADDR_ANY;
		setsockopt(sock->channel, IPPROTO_IP, IP_ADD_MEMBERSHIP,
			(char*) &g, sizeof(g));
	}
#endif

	/* The socket is ready */

	return(sock);

error_return:
	netlib_udp_close(sock);

	return NULL;
}

void netlib_udp_set_packet_loss(udp_socket sock, int percent)
{
	/* FIXME: We may want this behavior to be reproducible
	   but there isn't a portable reentrant random
	   number generator with good randomness.
	*/
	srandom(time(NULL));

	if (percent < 0)
		percent = 0;
	else if (percent > 100)
		percent = 100;
	sock->packetloss = percent;
}

/* Verify that the channel is in the valid range */
static int valid_channel(int channel)
{
	if ((channel < 0) || (channel >= NETLIB_MAX_UDPCHANNELS))
	{
		netlib_set_error("Invalid channel");
		return 0;
	}
	return 1;
}

/* Bind the address 'address' to the requested channel on the UDP socket.
   If the channel is -1, then the first unbound channel that has not yet
   been bound to the maximum number of addresses will be bound with
   the given address as it's primary address.
   If the channel is already bound, this new address will be added to the
   list of valid source addresses for packets arriving on the channel.
   If the channel is not already bound, then the address becomes the primary
   address, to which all outbound packets on the channel are sent.
   This function returns the channel which was bound, or -1 on error.
*/
int netlib_udp_bind(udp_socket sock, int channel, const ip_address *address)
{
	struct udp_channel *binding;

	if (sock == NULL)
	{
		netlib_set_error("Passed a NULL socket");
		return -1;
	}

	if (channel == -1)
	{
		for (channel = 0; channel < NETLIB_MAX_UDPCHANNELS; ++channel)
		{
			binding = &sock->binding[channel];
			if (binding->numbound < NETLIB_MAX_UDPADDRESSES)
				break;
		}
	}
	else
	{
		if (!valid_channel(channel))
			return -1;
		binding = &sock->binding[channel];
	}

	if (binding->numbound == NETLIB_MAX_UDPADDRESSES)
	{
		netlib_set_error("No room for new addresses");
		return -1;
	}

	binding->address[binding->numbound++] = *address;
	return channel;
}

/* Unbind all addresses from the given channel */
void netlib_udp_unbind(udp_socket sock, int channel)
{
	if ((channel >= 0) && (channel < NETLIB_MAX_UDPCHANNELS))
		sock->binding[channel].numbound = 0;
}

/* Get the primary IP address of the remote system associated with the
   socket and channel.
   If the channel is not bound, this function returns NULL.
*/
ip_address* netlib_udp_get_peer_address(udp_socket sock, int channel)
{
	ip_address* address;

	address = NULL;
	switch (channel)
	{
	case -1:
		/* Return the actual address of the socket */
		address = &sock->address;
		break;
	default:
		/* Return the address of the bound channel */
		if (valid_channel(channel) && (sock->binding[channel].numbound > 0))
			address = &sock->binding[channel].address[0];
		break;
	}

	return address;
}

/* Send a vector of packets to the the channels specified within the packet.
   If the channel specified in the packet is -1, the packet will be sent to
   the address in the 'src' member of the packet.
   Each packet will be updated with the status of the packet after it has
   been sent, -1 if the packet send failed.
   This function returns the number of packets sent.
*/
int netlib_udp_send_packets(udp_socket sock, udp_packet** packets, int npackets)
{
	int numsent, i, j;
	struct udp_channel* binding;
	int status;
	int sock_len;
	struct sockaddr_in sock_addr;

	if (sock == NULL)
	{
		netlib_set_error("Passed a NULL socket");
		return 0;
	}

	/* Set up the variables to send packets */
	sock_len = sizeof(sock_addr);

	numsent = 0;
	for (i = 0; i < npackets; ++i)
	{
		/* Simulate packet loss, if desired */
		if (sock->packetloss)
		{
			if ((random() % 100) <= sock->packetloss)
			{
				packets[i]->status = packets[i]->len;
				++numsent;
				continue;
			}
		}

		/* if channel is < 0, then use channel specified in sock */
		if (packets[i]->channel < 0)
		{
			sock_addr.sin_addr.s_addr = packets[i]->address.host;
			sock_addr.sin_port = packets[i]->address.port;
			sock_addr.sin_family = AF_INET;
			status = sendto(sock->channel, packets[i]->data, packets[i]->len, 0,
				(struct sockaddr*) &sock_addr, sock_len);
			if (status >= 0)
			{
				packets[i]->status = status;
				++numsent;
			}
		}
		else
		{
			/* Send to each of the bound addresses on the channel */
#ifdef DEBUG_NET
			printf("SDLNet_UDP_SendV sending packet to channel = %d\n", packets[i]->channel);
#endif

			binding = &sock->binding[packets[i]->channel];

			for (j = binding->numbound - 1; j >= 0; --j)
			{
				sock_addr.sin_addr.s_addr = binding->address[j].host;
				sock_addr.sin_port = binding->address[j].port;
				sock_addr.sin_family = AF_INET;
				status = sendto(sock->channel, packets[i]->data, packets[i]->len, 0,
					(struct sockaddr*) &sock_addr, sock_len);
				if (status >= 0)
				{
					packets[i]->status = status;
					++numsent;
				}
			}
		}
	}

	return numsent;
}

int netlib_udp_send(udp_socket sock, int channel, udp_packet* packet)
{
	/* This is silly, but...
	   (Very reassuring comment)
	*/
	packet->channel = channel;
	return netlib_udp_send_packets(sock, &packet, 1);
}

/* Returns true if a socket is has data available for reading right now */
static int socket_ready(SOCKET sock)
{
	int retval = 0;
	struct timeval tv;
	fd_set mask;

	/* Check the file descriptors for available data */
	do
	{
		netlib_set_last_error(0);

		/* Set up the mask of file descriptors */
		FD_ZERO(&mask);
		FD_SET(sock, &mask);

		/* Set up the timeout */
		tv.tv_sec = 0;
		tv.tv_usec = 0;

		/* Look! */
		retval = select(sock + 1, &mask, NULL, NULL, &tv);
	} while (netlib_get_last_error() == EINTR);

	return retval == 1;
}

/* Receive a vector of pending packets from the UDP socket.
   The returned packets contain the source address and the channel they arrived
   on.  If they did not arrive on a bound channel, the the channel will be set
   to -1.
   This function returns the number of packets read from the network, or -1
   on error.  This function does not block, so can return 0 packets pending.
*/
extern int netlib_udp_recv_packets(udp_socket sock, udp_packet** packets)
{
	int numrecv, i, j;
	struct udp_channel* binding;
	socklen_t sock_len;
	struct sockaddr_in sock_addr;

	if (sock == NULL)
		return 0;

	numrecv = 0;
	while (packets[numrecv] && socket_ready(sock->channel))
	{
		udp_packet* packet;

		packet = packets[numrecv];

		sock_len = sizeof(sock_addr);
		packet->status = recvfrom(sock->channel, packet->data, packet->maxlen, 0,
			(struct sockaddr*) &sock_addr, &sock_len);

		if (packet->status >= 0)
		{
			packet->len = packet->status;
			packet->address.host = sock_addr.sin_addr.s_addr;
			packet->address.port = sock_addr.sin_port;
			packet->channel = -1;

			for (i = (NETLIB_MAX_UDPCHANNELS - 1); i >= 0; --i)
			{
				binding = &sock->binding[i];

				for (j = binding->numbound - 1; j >= 0; --j)
				{
					if ((packet->address.host == binding->address[j].host) &&
						(packet->address.port == binding->address[j].port))
					{
						packet->channel = i;
						goto foundit; /* break twice */
					}
				}
			}
		foundit:
			++numrecv;
		}

		else
		{
			packet->len = 0;
		}
	}

	sock->ready = 0;

	return(numrecv);
}

/* Receive a single packet from the UDP socket.
   The returned packet contains the source address and the channel it arrived
   on.  If it did not arrive on a bound channel, the the channel will be set
   to -1.
   This function returns the number of packets read from the network, or -1
   on error.  This function does not block, so can return 0 packets pending.
*/
int netlib_udp_recv(udp_socket sock, udp_packet* packet)
{
	udp_packet* packets[2];

	/* Receive a packet array of 1 */
	packets[0] = packet;
	packets[1] = NULL;
	return netlib_udp_recv_packets(sock, packets);
}

/* Close a UDP network socket */
extern void netlib_udp_close(udp_socket sock)
{
	if (sock != NULL) {
		if (sock->channel != INVALID_SOCKET)
			closesocket(sock->channel);
		
		free(sock);
	}
}
