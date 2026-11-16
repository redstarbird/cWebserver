#include "../include/cWebServer.h"
#include "../src/webserver_internal.h"
#include "testing.h"

void testDestroyNullServer(void)
{
    CWebServer_DestroyServer(NULL);
}

void testPortReuseAfterDestroy(void)
{
    struct CWebServer_Server *server1 = CWebServer_CreateServer(8080, 8443, 1024, NULL, NULL, NULL);
    ASSERT_TRUE(server1 != NULL, "Initial server creation failed.");

    CWebServer_DestroyServer(server1);

    struct CWebServer_Server *server2 = CWebServer_CreateServer(8080, 8443, 1024, NULL, NULL, NULL);
    ASSERT_TRUE(server2 != NULL, "Second server creation failed with reused port.");

    CWebServer_DestroyServer(server2);
}

void testWebserverMultipleRestarts()
{
    CWebServer_Server *server = CWebServer_CreateServer(8081, 8455, 1024, NULL, NULL, NULL);

    for (int i = 0; i < 3; i++)
    {
        CWebServer_StartServerAsync(server);
        ASSERT_TRUE(server->running, "Server was not running.");

        CWebServer_StopServer(server);
        ASSERT_EQUALS(false, server->running, "Server was running after being stopped.");
    }
}

void testStartStopWebserver()
{
    struct CWebServer_Server *server = CWebServer_CreateServer(8080, 8443, 1024, "cert.pem", "key.pem", NULL);

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
        servers[i] = CWebServer_CreateServer(8001 + i, 8443 + i, 1024 + i, NULL, NULL, NULL);
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
    struct CWebServer_Server *server = CWebServer_CreateServer(8001, 8002, 1024, NULL, NULL, NULL);

    ASSERT_EQUALS(server->HTTPPort, 8001, "The server's HTTP port was not set correctly.");

    ASSERT_EQUALS(server->HTTPSPort, 8002, "The server's HTTPS port was not set correctly.");

    ASSERT_EQUALS(server->maxQueued, 1024, "The server's maximum queued value was not set correctly.");

    CWebServer_DestroyServer(server);
}

void testCreateServerMallocFails(void)
{
    cws_error_t err;
    mock_malloc_fail = true;
    struct CWebServer_Server *server = CWebServer_CreateServer(8001, 8002, 1024, NULL, NULL, &err);
    mock_malloc_fail = false;

    ASSERT_TRUE(server == NULL, "Server was not NULL when malloc failed.");

    ASSERT_EQUALS(CWS_ERR_MEMORY, err, "Error output was not set to memory error.");
}

void testCreateServerSocketFails(void)
{
    cws_error_t err;
    mock_socket_fail = true;
    struct CWebServer_Server *server = CWebServer_CreateServer(8001, 8002, 1024, NULL, NULL, &err);
    mock_socket_fail = false;

    ASSERT_TRUE(server == NULL, "Server was not NULL when socket creation failed.");

    ASSERT_EQUALS(CWS_ERR_SOCKET, err, "Error output was not set to socket error.");
}

void testCreateServerSocketBindFails(void)
{
    cws_error_t err;
    mock_bind_fail = true;
    struct CWebServer_Server *server = CWebServer_CreateServer(8001, 8002, 1024, NULL, NULL, &err);
    mock_bind_fail = false;

    ASSERT_TRUE(server == NULL, "Server was not NULL when socket bind failed.");

    ASSERT_EQUALS(CWS_ERR_SOCKET, err, "Error output was not set to socket error.");
}

void testCreateServerSocketListenFails(void)
{
    cws_error_t err;
    mock_listen_fail = true;
    struct CWebServer_Server *server = CWebServer_CreateServer(8001, 8002, 1024, NULL, NULL, &err);
    mock_listen_fail = false;

    ASSERT_TRUE(server == NULL, "Server was not NULL when socket listen failed.");

    ASSERT_EQUALS(CWS_ERR_SOCKET, err, "Error output was not set to socket error.");
}

void testCreateServerSSLCTXnewFails(void)
{
    cws_error_t err;
    mock_ssl_fail = true;
    struct CWebServer_Server *server = CWebServer_CreateServer(8001, 8002, 1024, NULL, NULL, &err);
    mock_ssl_fail = false;

    ASSERT_TRUE(server == NULL, "Server was not NULL when SSL context creation failed.");

    ASSERT_EQUALS(CWS_ERR_TLS, err, "Error output was not set to TLS error.");
}

int main(void)
{
    testCreateWebserver();
    testCreateMultipleWebservers();
    testStartStopWebserver();
    testCreateWebserverOnUsedPortReturnsNull();
    testCreateServerMallocFails();
    testCreateServerSocketFails();
    testCreateServerSocketBindFails();
    testCreateServerSocketListenFails();
    testCreateServerSSLCTXnewFails();
    return 0;
}