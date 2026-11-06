// Standard io
#include <stdio.h>
// Standard libary
#include <stdlib.h>
// Standard string functions
#include <string.h>

// Socket functions for networking
#include <sys/socket.h>
// Types for socket functions
#include <sys/types.h>

// Functions for internet addresses and host names
#include <netdb.h>

// Functions and structs for internet addresses and ports
#include <netinet/in.h>

// SSL functions for encrypted HTTPS connections
#include <openssl/ssl.h>

// For getting the current working directory and the function for closing sockets
#include <unistd.h>

// For error handling
#include <errno.h>

// Time functions
#include <time.h>

// For running the HTTP and HTTPS servers on separate threads
#include <pthread.h>

// For PATH_MAX definition
#include <linux/limits.h>

// Include the internal header files for the webserver
#include "./internal_codes.h"
#include "./util/file_util.h"
#include "./content_types.h"

// Global definitions for the webserver
// Port numbers for the HTTP and HTTP servers
#define HTTPPORT 80
#define HTTPSPORT 443

// Max number of queued connections for the listener sockets
#define MAXQUEUED 1000

// Path for the static files for the webserver
#define STATICPATH "/public"

// Global variable for the current working directory and its length
char CWD[PATH_MAX];
int CWD_len;

// Array storing HTTP status codes and their corresponding messages
const char *HTTP_Status_Codes[600] = {
    [200] = "OK",
    [301] = "Moved Permanently",
    [400] = "Bad Request",
    [404] = "Not Found",
    [500] = "Internal Server Error"};

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
    SSL *SSL_ctx;

    // Threads
    pthread_t HTTP_Thread;
    pthread_t HTTPS_Thread;

    // The maximum nujmber of requests that can be queued per socket
    uint32_t maxQueued;

    // Stores whether the server is running
    bool running;
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
} HTTPResponse;

// Enum for the different HTTP request methods
enum HTTPRequestMethod
{
    GET,
    HEAD,
    POST,
    DELETE
} HTTPRequestMethod;

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
} HTTPRequest;