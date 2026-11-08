#ifndef WEBSERVER_INTERNAL_H
#define WEBSERVER_INTERNAL_H
// Standard io
#include <stdio.h>
// Standard libary
#include <stdlib.h>
// Standard string functions
#include <string.h>

#include "./platform/platform.h"

// SSL functions for encrypted HTTPS connections
#include <openssl/ssl.h>

// For error handling
#include <errno.h>

// Time functions
#include <time.h>

// Public library include
#include "../include/cWebServer.h"

// Include the internal header files for the webserver
#include "./internal_codes.h"
#include "./util/file_util.h"
#include "./content_types.h"

#define PATH_MAX 4096

// Array storing HTTP status codes and their corresponding messages
extern const char *HTTP_Status_Codes[600];

struct CWebServer_Server
{
    // Socket file descriptors
    int HTTP_Socket;
    int HTTPS_Socket;

    // Port numbers
    uint16_t HTTPPort;
    uint16_t HTTPSPort;

    // Root path from which to serve static paths from
    char rootPath[PATH_MAX];
    size_t rootPathLen;

    // SSL context
    SSL_CTX *SSL_ctx;

    // Threads
    cws_thread_t HTTP_Thread;
    cws_thread_t HTTPS_Thread;

    // The maximum nujmber of requests that can be queued per socket
    uint32_t maxQueued;

    // Stores whether the server is running
    volatile bool running;
};

// Struct for storing the content and metadata for a HTTP response
struct HTTPResponse
{
    // Status code and message
    unsigned short int statusCode;
    char *statusMessage;
    // Stores the content type of the response
    char *contentType;
    // Stores the content of the response
    char *content;
    // Stores the length of the content
    int contentLength;
    // Stores the date/timestamp of the response
    time_t date;
    // Stores a redirect location header
    char *location;
};

// Enum for the different HTTP request methods
enum HTTPRequestMethod
{
    GET,
    HEAD,
    POST,
    DELETE
};

// Struct for storing data about a HTTP request
struct HTTPRequest
{
    // The HTTP request method
    enum HTTPRequestMethod method;

    // The exact path requested from the client
    char *relativePath;

    // The absolute path of the content requested by the client
    char *absolutePath;

    // Request body (empty for GET and HEAD requests)
    char *body;

    // SSL session for the request
    SSL *sslSession;
};

#endif