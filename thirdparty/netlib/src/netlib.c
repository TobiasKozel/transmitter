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
#include <string.h>
#include <stdarg.h>

const netlib_version* netlib_get_version()
{
	static netlib_version version;
	NETLIB_VERSION(&version);
	return &version;
}

static int netlib_started = 0;

#ifndef _WIN32
#include <signal.h>

int netlib_get_last_error(void)
{
	return errno;
}

void netlib_set_last_error(int err)
{
	errno = err;
}
#endif

static char errorbuf[1024];

void NETLIB_CALL netlib_set_error(const char* fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);
	vsnprintf(errorbuf, sizeof(errorbuf), fmt, argp);
	va_end(argp);
}

const char* NETLIB_CALL netlib_get_error()
{
	return errorbuf;
}

DECLSPEC netlib_byte_buf* NETLIB_CALL netlib_alloc_byte_buf(uint8_t size)
{
	netlib_byte_buf* buf = NULL;
	int error = 1;
	buf = (netlib_byte_buf*) malloc(sizeof(netlib_byte_buf));

	if (buf)
	{
		buf->length = size;
		buf->data = (uint8_t*) malloc(size);
		buf->read_pos = 0;
		buf->write_pos = 0;
		if (buf->data)
			error = 0;
	}

	if (error)
	{
		netlib_set_error("Out of memory");
		netlib_free_byte_buf(buf);
		buf = NULL;
	}

	return buf;
}

void netlib_free_byte_buf(netlib_byte_buf* buf)
{
	if (buf)
	{
		free(buf->data);
		free(buf);
	}
}

int netlib_write_uint8(netlib_byte_buf* buf, uint8_t val)
{
	if (!buf)
	{
		return -1;
	}

	if (buf->write_pos + sizeof(uint8_t) > buf->length)
	{
		netlib_set_error("Not enough space in byte data");
		return -1;
	}

	buf->data[buf->write_pos++] = val;
	return 1;
}

int netlib_write_uint16(netlib_byte_buf* buf, uint16_t val)
{
	if (!buf)
	{
		return -1;
	}

	if (buf->write_pos + sizeof(uint16_t) > buf->length)
	{
		netlib_set_error("Not enough space in byte data");
		return -1;
	}

	buf->data[buf->write_pos++] = (val >> 8) & 0xff;
	buf->data[buf->write_pos++] = val & 0xff;
	return 1;
}

int netlib_write_uint32(netlib_byte_buf* buf, uint32_t val)
{
	if (!buf)
	{
		return -1;
	}

	if (buf->write_pos + sizeof(uint32_t) > buf->length)
	{
		netlib_set_error("Not enough space in byte data");
		return -1;
	}

	buf->data[buf->write_pos++] = (uint8_t) (val >> 24) & 0xff;
	buf->data[buf->write_pos++] = (uint8_t) (val >> 16) & 0xff;
	buf->data[buf->write_pos++] = (uint8_t) (val >> 8) & 0xff;
	buf->data[buf->write_pos++] = (uint8_t) val & 0xff;

	return 1;
}

int netlib_write_float(netlib_byte_buf* buf, float val)
{
	if (!buf)
	{
		return -1;
	}

	if (buf->write_pos + sizeof(float) > buf->length)
	{
		netlib_set_error("Not enough space in byte data");
		return -1;
	}
	
	union {
		float f;
		unsigned char bytes[sizeof(float)];
	} float2byte;
	
	float2byte.f = val;
	memcpy(buf->data + buf->write_pos, float2byte.bytes, sizeof(float));
	buf->write_pos += sizeof(float);
	return 1;
}

int netlib_read_uint8(netlib_byte_buf* buf, uint8_t* val)
{
	if (!buf)
	{
		return -1;
	}

	if (buf->read_pos + sizeof(uint8_t) > buf->length)
	{
		netlib_set_error("Not enough space in byte data");
		return -1;
	}

	*val = buf->data[buf->read_pos++];
	return 1;
}

int netlib_read_uint16(netlib_byte_buf* buf, uint16_t* val)
{
	if (!buf)
	{
		return -1;
	}

	if (buf->read_pos + sizeof(uint16_t) > buf->length)
	{
		netlib_set_error("Not enough space in byte data");
		return -1;
	}

	*val = (buf->data[buf->read_pos++] << 8);
	*val |= buf->data[buf->read_pos++] & 0xff;

	return 1;
}

int netlib_read_uint32(netlib_byte_buf* buf, uint32_t* val)
{
	if (!buf)
	{
		return -1;
	}

	if (buf->write_pos + sizeof(uint32_t) > buf->length)
	{
		netlib_set_error("Not enough space in byte data");
		return -1;
	}
	
	*val = 0;
	*val = buf->data[buf->read_pos++] << 24;
	*val |= buf->data[buf->read_pos++] << 16;
	*val |= buf->data[buf->read_pos++] << 8;
	*val |= buf->data[buf->read_pos++] & 0xff;

	return 1;
}

int netlib_read_float(netlib_byte_buf* buf, float* val)
{
	if (!buf)
	{
		return -1;
	}

	if (buf->write_pos + sizeof(float) > buf->length)
	{
		netlib_set_error("Not enough space in byte data");
		return -1;
	}

	union {
		unsigned char bytes[sizeof(float)];
		float f;
	} float2byte;

	memcpy(float2byte.bytes, buf->data + buf->read_pos, sizeof(float));
	*val = float2byte.f;
	buf->read_pos += sizeof(float);

	return 1;
}

int netlib_init(void)
{
	if (!netlib_started)
	{
#ifdef __USE_W32_SOCKETS
		/* Start up the windows networking */
		WORD version_wanted = MAKEWORD(1, 1);
		WSADATA wsaData;

		if (WSAStartup(version_wanted, &wsaData) != 0)
		{
			netlib_set_error("Couldn't initialize Winsock 1.1\n");
			return(-1);
		}
#else
		/* SIGPIPE is generated when a remote socket is closed */
		void(*handler)(int);
		handler = signal(SIGPIPE, SIG_IGN);
		if (handler != SIG_DFL)
			signal(SIGPIPE, handler);
#endif
	}

	++netlib_started;
	return 0;
}

void netlib_quit(void)
{
	if (netlib_started == 0)
		return;
	
	if (--netlib_started == 0)
	{
#ifdef __USE_W32_SOCKETS
		/* Clean up windows networking */
		if (WSACleanup() == SOCKET_ERROR)
		{
			if (WSAGetLastError() == WSAEINPROGRESS)
			{
#ifndef _WIN32_WCE
				WSACancelBlockingCall();
#endif
				WSACleanup();
			}
		}
#else
		/* Restore the SIGPIPE handler */
		void(*handler)(int);
		handler = signal(SIGPIPE, SIG_DFL);
		if (handler != SIG_IGN)
			signal(SIGPIPE, handler);
#endif
	}
}

int netlib_resolve_host(ip_address* address, const char* host, uint16_t port)
{
	int retval = 0;

	/* Perform the actual host resolution */
	if (host == NULL)
	{
		address->host = INADDR_ANY;
	}
	else
	{
		address->host = inet_addr(host);
		if (address->host == INADDR_NONE)
		{
			struct hostent *hp;
			hp = gethostbyname(host);
			if (hp)
			{
				memcpy(&address->host, hp->h_addr, hp->h_length);
			}
			else
			{
				retval = -1;
				netlib_set_error("failed to get host from '%s'", host);
			}
		}
	}
	address->port = netlib_read16(&port);

	/* Return the status */
	return retval;
}

/* Resolve an ip address to a host name in canonical form.
   If the ip couldn't be resolved, this function returns NULL,
   otherwise a pointer to a static data containing the hostname
   is returned.  Note that this function is not thread-safe.

   Written by Miguel Angel Blanch.
   Main Programmer of Arianne RPG.
   http://come.to/arianne_rpg
*/
const char* netlib_resolve_ip(const ip_address* ip)
{
	struct hostent *hp;
	struct in_addr in;

	hp = gethostbyaddr((const char*) &ip->host, sizeof(ip->host), AF_INET);
	
	if (hp != NULL)
		return hp->h_name;

	in.s_addr = ip->host;
	return inet_ntoa(in);
}

int netlib_get_local_addresses(ip_address* addresses, int maxcount)
{
	int count = 0;
#ifdef SIOCGIFCONF
	/* Defined on Mac OS X */
#ifndef _SIZEOF_ADDR_IFREQ
#define _SIZEOF_ADDR_IFREQ sizeof
#endif
	SOCKET sock;
	struct ifconf conf;
	char data[4096];
	struct ifreq *ifr;
	struct sockaddr_in *sock_addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock == INVALID_SOCKET) {
		return 0;
	}

	conf.ifc_len = sizeof(data);
	conf.ifc_buf = (caddr_t)data;
	if (ioctl(sock, SIOCGIFCONF, &conf) < 0) {
		closesocket(sock);
		return 0;
	}

	ifr = (struct ifreq*)data;
	while ((char*)ifr < data + conf.ifc_len) {
		if (ifr->ifr_addr.sa_family == AF_INET) {
			if (count < maxcount) {
				sock_addr = (struct sockaddr_in*)&ifr->ifr_addr;
				addresses[count].host = sock_addr->sin_addr.s_addr;
				addresses[count].port = sock_addr->sin_port;
			}
			++count;
		}
		ifr = (struct ifreq*)((char*)ifr + _SIZEOF_ADDR_IFREQ(*ifr));
	}
	closesocket(sock);
#elif defined(WIN32)
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter;
	PIP_ADDR_STRING pAddress;
	DWORD dwRetVal = 0;
	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	pAdapterInfo = (IP_ADAPTER_INFO*) malloc(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL)
		return 0;
	
	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == ERROR_BUFFER_OVERFLOW)
	{
		pAdapterInfo = (IP_ADAPTER_INFO*) realloc(pAdapterInfo, ulOutBufLen);
		if (pAdapterInfo == NULL)
			return 0;

		dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
	}

	if (dwRetVal == NO_ERROR)
	{
		for (pAdapter = pAdapterInfo; pAdapter; pAdapter = pAdapter->Next)
		{
			for (pAddress = &pAdapter->IpAddressList; pAddress; pAddress = pAddress->Next)
			{
				if (count < maxcount)
				{
					addresses[count].host = inet_addr(pAddress->IpAddress.String);
					addresses[count].port = 0;
				}
				++count;
			}
		}
	}
	free(pAdapterInfo);
#endif
	return count;
}