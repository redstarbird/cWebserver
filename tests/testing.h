#ifndef TESTING_H
#define TESTING_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/** @brief Assert that a condition is true */
#define ASSERT_TRUE(condition, message)                                         \
    do                                                                          \
    {                                                                           \
        if (!(condition))                                                       \
        {                                                                       \
            fprintf(stderr, "[FAIL] %s:%d: %s\n", __FILE__, __LINE__, message); \
            exit(1);                                                            \
        }                                                                       \
    } while (0)

/** @brief Assert that two values are equal */
#define ASSERT_EQUALS(expected, actual, message)                                                                                      \
    do                                                                                                                                \
    {                                                                                                                                 \
        if ((expected) != (actual))                                                                                                   \
        {                                                                                                                             \
            fprintf(stderr, "[FAIL] %s:%d: %s (Expected %d, got %d)\n", __FILE__, __LINE__, message, (int)(expected), (int)(actual)); \
            exit(1);                                                                                                                  \
        }                                                                                                                             \
    } while (0)

// =========
// | Mocks |
// =========

#include <openssl/ssl.h>
#include "../src/platform/platform.h"

// Mock control flags
bool mock_malloc_fail = false;
bool mock_socket_fail = false;
bool mock_bind_fail = false;
bool mock_listen_fail = false;
bool mock_ssl_fail = false;

// These call the real functions once wrapped
extern void *__real_malloc(size_t size);
extern int __real_socket(int domain, int type, int protocol);
extern int __real_bind(int fd, const struct sockaddr *addr, socklen_t len);
extern int __real_listen(int fd, int n);
extern SSL_CTX *__real_SSL_CTX_new(const SSL_METHOD *method);

void *__wrap_malloc(size_t size)
{
    if (mock_malloc_fail)
    {
        return NULL;
    }
    return __real_malloc(size);
}

int __wrap_socket(int af, int type, int protocol)
{
    if (mock_socket_fail)
    {
        return CWS_INVALID_SOCKET;
    }
    return __real_socket(af, type, protocol);
}

int __wrap_bind(int fd, const struct sockaddr *addr, socklen_t len)
{
    if (mock_bind_fail)
    {
        return -1;
    }
    return __real_bind(fd, addr, len);
}

int __wrap_listen(int fd, int n)
{
    if (mock_listen_fail)
    {
        return -1;
    }
    return __real_listen(fd, n);
}

SSL_CTX *__wrap_SSL_CTX_new(const SSL_METHOD *method)
{
    if (mock_ssl_fail)
    {
        return NULL;
    }
    return __real_SSL_CTX_new(method);
}

#endif // TESTING_H