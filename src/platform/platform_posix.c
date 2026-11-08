#include "./platform.h"

int cws_CloseSocket(cws_socket_t s)
{
    return close(s);
}

int cws_GetSocketError(void)
{
    return errno;
}