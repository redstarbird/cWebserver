#include "../include/cWebServer.h"
#include "../src/webserver_internal.h"
#include "testing.h"

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
    return 0;
}