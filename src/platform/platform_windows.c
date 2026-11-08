#include "./platform.h"

int cws_CloseSocket(cws_socket_t s)
{
    return closesocket(s);
}

int cws_GetSocketError(void)
{
    return WSAGetLastError();
}

struct WindowsThreadArgs
{
    cws_thread_func_t func;
    void *arg;
};

static unsigned __stdcall WindowsThreadAdapter(void *param)
{
    struct WindowsThreadArgs args = *(struct WindowsThreadArgs *)param;
    free(param);

    args.func(args.arg);
    return 0;
}

int cws_thread_create(cws_thread_t *thread, cws_thread_func_t func, void *arg)
{
    struct WindowsThreadArgs *threadArgs = malloc(sizeof(struct WindowsThreadArgs));

    if (!threadArgs)
    {
        return -1;
    }

    threadArgs->arg = arg;
    threadArgs->func = func;

    uintptr_t handle = (cws_thread_t)_beginthreadex(NULL, 0, &WindowsThreadAdapter, (void *)threadArgs, 0, NULL);
    if (handle == 0)
    {
        return -1;
    }

    *thread = (HANDLE)handle;
    return 0;
}

int cws_thread_join(cws_thread_t thread)
{
    WaitForSingleObject(thread, INFINITE);

    CloseHandle(thread);
    return 0;
}