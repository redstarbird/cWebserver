// Contains abstractions for everything that is platform specific
#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32 // Windows

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// Sockets
#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET cws_socket_t;

#else // POSIX

// Sockets

#include <sys/socket.h>

// Types for socket functions
#include <sys/types.h>

typedef int cws_socket_t;

#endif

int cws_CloseSocket(cws_socket_t s);

#endif