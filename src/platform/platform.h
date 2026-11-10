// Contains abstractions for everything that is platform specific
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdlib.h>
#include <stdint.h>

#ifdef _WIN32 // Windows

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <process.h>

// Sockets
#include <winsock2.h>
#include <ws2tcpip.h>

typedef SOCKET cws_socket_t;

#define CWS_SOCKET_ERROR SOCKET_ERROR
#define CWS_INVALID_SOCKET INVALID_SOCKET

typedef HANDLE cws_thread_t;

#include <direct.h>
#define getcwd _getcwd

#else // POSIX

// Sockets

#include <sys/socket.h>

// Types for socket functions
#include <sys/types.h>

#include <errno.h>

#define CWS_SOCKET_ERROR (-1)
#define CWS_INVALID_SOCKET (-1)

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

typedef pthread_t cws_thread_t;

#endif

int cws_CloseSocket(cws_socket_t s);
int cws_GetSocketError(void);

typedef void *(*cws_thread_func_t)(void *arg);
int cws_thread_create(cws_thread_t *thread, cws_thread_func_t func, void *arg);
int cws_thread_join(cws_thread_t thread);

#endif