#ifndef CWEBSERVER_H
#define CWEBSERVER_H

#include <stdint.h>

typedef struct CWebServer_Server CWebServer_Server;
typedef struct HTTPRequest CWebServer_Request;
typedef struct HTTPResponse CWebServer_Response;

CWebServer_Server *CWebServer_CreateServer(uint16_t HTTP_Port, uint16_t HTTPS_Port, uint32_t maxQueued, const char *certPath, const char *keyPath);
void CWebServer_DestroyServer(CWebServer_Server *server);

#endif