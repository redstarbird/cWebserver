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
