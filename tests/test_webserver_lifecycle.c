#include "../include/cWebServer.h"
#include "../src/webserver_internal.h"
#include "testing.h"

void testStartStopWebserver()
{
    struct CWebServer_Server *server = CWebServer_CreateServer(8080, 8443, 1024, "cert.pem", "key.pem");

    CWebServer_StartServerAsync(server);

    ASSERT_TRUE(server->running, "Server was not set as running.");

    CWebServer_StopServer(server);

    ASSERT_EQUALS(server->running, false, "Server was still set as running after stop was called");

    CWebServer_DestroyServer(server);
}

void testCreateMultipleWebservers()
{
    const int serverNum = 5;

    struct CWebServer_Server **servers = malloc(sizeof(CWebServer_Server *) * serverNum);

    ASSERT_TRUE(servers != NULL, "Failed to allocate servers array.");

    for (int i = 0; i < serverNum; i++)
    {
        servers[i] = CWebServer_CreateServer(8001 + i, 8443 + i, 1024 + i, NULL, NULL);
    }

    for (int i = 0; i < serverNum; i++)
    {
        ASSERT_TRUE(servers[i] != NULL, "Server instance was NULL.");
        ASSERT_TRUE(servers[i]->HTTP_Socket != CWS_INVALID_SOCKET, "HTTP socket was invalid.");
        ASSERT_TRUE(servers[i]->HTTPS_Socket != CWS_INVALID_SOCKET, "HTTP socket was invalid.");
        ASSERT_TRUE(servers[i]->SSL_ctx != NULL, "SSL context was not initialised.");

        ASSERT_EQUALS(8001 + i, servers[i]->HTTPPort, "The server's HTTP port was not set correctly.");
        ASSERT_EQUALS(8443 + i, servers[i]->HTTPSPort, "The server's HTTPS port was not set correctly.");
        ASSERT_EQUALS(1024 + i, servers[i]->maxQueued, "The server's maximum queued value was not set correctly.");

        CWebServer_DestroyServer(servers[i]);
    }

    free(servers);
}

void testCreateWebserver()
{
    struct CWebServer_Server *server = CWebServer_CreateServer(8001, 8002, 1024, NULL, NULL);

    ASSERT_EQUALS(server->HTTPPort, 8001, "The server's HTTP port was not set correctly.");

    ASSERT_EQUALS(server->HTTPSPort, 8002, "The server's HTTPS port was not set correctly.");

    ASSERT_EQUALS(server->maxQueued, 1024, "The server's maximum queued value was not set correctly.");

    CWebServer_DestroyServer(server);
}

int main(void)
{
    testCreateWebserver();
    testCreateMultipleWebservers();
    testStartStopWebserver();
    return 0;
}