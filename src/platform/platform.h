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

// For getting the current working directory and the function for closing sockets
#include <unistd.h>

typedef int cws_socket_t;

// Functions for internet addresses and host names
#include <netdb.h>

// Functions and structs for internet addresses and ports
#include <netinet/in.h>

// Threads

// For running the HTTP and HTTPS servers on separate threads
#include <pthread.h>

#include <poll.h>

#endif

int cws_CloseSocket(cws_socket_t s);

#endif