#include "../include/cWebServer.h"
#include "../src/webserver_internal.h"
#include "testing.h"

void testStartStopWebserver()
{
    struct CWebServer_Server *server = CWebServer_CreateServer(8080, 8443, 1024, "../cert.pem", "../key.pem");

    CWebServer_StartServerAsync(server);

    ASSERT_TRUE(server->running, "Server was not set as running.");

    CWebServer_StopServer(server);

    ASSERT_EQUALS(server->running, false, "Server was still set as running after stop was called");

    CWebServer_DestroyServer(server);
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
    testStartStopWebserver();
    return 0;
}