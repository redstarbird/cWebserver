#include "./platform.h"

int cws_CloseSocket(cws_socket_t s)
{
    return close(s);
}

int cws_GetSocketError(void)
{
    return errno;
}

int cws_thread_create(cws_thread_t *thread, cws_thread_func_t func, void *arg)
{
    return pthread_create(thread, NULL, func, arg);
}

int cws_thread_join(cws_thread_t thread)
{
    return pthread_join(thread, NULL);
}

int cws_poll(struct pollfd *fds, unsigned int nfds, int timeout_ms)
{
    return poll(fds, nfds, timeout_ms);
}