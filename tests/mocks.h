#ifndef CWS_MOCKS_H
#define CWS_MOCKS_H

#include <stdbool.h>
#include <stddef.h>
#include <openssl/ssl.h>
#include "../src/platform/platform.h"

// Mock control flags
extern bool mock_malloc_fail;
extern bool mock_socket_fail;
extern bool mock_bind_fail;
extern bool mock_listen_fail;
extern bool mock_ssl_fail;

// Mock function declaration
void *cws_mock_malloc(size_t size);
int cws_mock_socket(int af, int type, int protocol);
int cws_mock_bind(int fd, const struct sockaddr *addr, socklen_t len);
int cws_mock_listen(int fd, int n);
SSL_CTX *cws_mock_SSL_CTX_new(const SSL_METHOD *method);

// Redefine mocked functions
#if defined(CWS_TESTING) && !defined(CWS_NO_MOCK_MACROS)
#define malloc(size) cws_mock_malloc(size);
#define socket(af, type, proto) cws_mock_socket(af, type, proto)
#define bind(fd, addr, len) cws_mock_bind(fd, addr, len)
#define listen(fd, n) cws_mock_listen(fd, n)
#define SSL_CTX_new(method) cws_mock_SSL_CTX_new(method)
#endif

#endif // CWS_MOCKS_H