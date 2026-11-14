#ifndef CWEBSERVER_H
#define CWEBSERVER_H

#include <stdint.h>

typedef struct CWebServer_Server CWebServer_Server;
typedef struct HTTPRequest CWebServer_Request;
typedef struct HTTPResponse CWebServer_Response;

typedef enum
{
    WS_OK = 0,
    WS_ERR_SOCKET = -1,
    WS_ERR_TLS = -3,
    WS_ERR_MEMORY = -4,
    WS_ERR_INVALID_ARG = -5,
    WS_ERR_THREAD = -6,
} ws_error_t;

CWebServer_Server *CWebServer_CreateServer(uint16_t HTTP_Port, uint16_t HTTPS_Port, uint32_t maxQueued, const char *certPath, const char *keyPath);
void CWebServer_DestroyServer(CWebServer_Server *server);
void CWebServer_StartServerAsync(CWebServer_Server *server);
void CWebServer_StartServerWait(CWebServer_Server *server);
void CWebServer_StopServer(CWebServer_Server *server);

#endif