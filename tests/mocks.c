#define CWS_NO_MOCK_MACROS
#include "mocks.h"

bool mock_malloc_fail = false;
bool mock_socket_fail = false;
bool mock_bind_fail = false;
bool mock_listen_fail = false;
bool mock_ssl_fail = false;

void *cws_mock_malloc(size_t size)
{
    if (mock_malloc_fail)
    {
        return NULL;
    }
    return malloc(size);
}

int cws_mock_socket(int af, int type, int protocol)
{
    if (mock_socket_fail)
    {
        return CWS_INVALID_SOCKET;
    }
    return socket(af, type, protocol);
}

int cws_mock_bind(int fd, const struct sockaddr *addr, socklen_t len)
{
    if (mock_bind_fail)
    {
        return -1;
    }
    return bind(fd, addr, len);
}

int cws_mock_listen(int fd, int n)
{
    if (mock_listen_fail)
    {
        return -1;
    }
    return listen(fd, n);
}

SSL_CTX *cws_mock_SSL_CTX_new(const SSL_METHOD *method)
{
    if (mock_ssl_fail)
    {
        return NULL;
    }
    return SSL_CTX_new(method);
}
