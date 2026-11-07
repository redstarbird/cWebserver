#include "./webserver_internal.h"

// Function to create a new listener socket on a given port
int createListenerSocket(int port, uint32_t maxQueued)
{

    // Setup server address structure
    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    // Create the socket file descriptor and check for any errors
    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1)
    {
        printf("An error occurred when attempting to create socket, errno: %d\n", errno);
        exit(1);
    }

    // Bind the socket to the server address and check for any errors
    if (bind(socketfd, (const struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
    {
        printf("An error occurred when attempting to bind socket to server address, errno: %d\n", errno);
        // Close the socket on error
        close(socketfd);
        exit(1);
    }

    // Prepare the socket to listen and accept connections
    if (listen(socketfd, maxQueued) == -1)
    {
        printf("An error occurred when attempting to call listen on socket, errno: %d\n", errno);
        // Close the socket on error
        close(socketfd);
        exit(1);
    }
    return socketfd;
}

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

const char *HTTP_Status_Codes[600] = {
    [200] = "OK",
    [301] = "Moved Permanently",
    [400] = "Bad Request",
    [404] = "Not Found",
    [500] = "Internal Server Error"};

CWebServer_Server *CWebServer_CreateServer(uint16_t HTTP_Port, uint16_t HTTPS_Port, uint32_t maxQueued)
{
    struct CWebServer_Server *server = malloc(sizeof(struct CWebServer_Server));

    server->HTTP_Socket = createListenerSocket(HTTP_Port, maxQueued);
    server->HTTPS_Socket = createListenerSocket(HTTPS_Port, maxQueued);

    server->HTTPPort = HTTP_Port;
    server->HTTPSPort = HTTPS_Port;

    server->rootPath[0] = '\0';
    server->rootPathLen = 0;

    server->SSL_ctx = SSL_CTX_new(TLS_server_method());

    server->maxQueued = maxQueued;

    server->running = false;

    return server;
}

void CWebServer_DestroyServer(CWebServer_Server *server)
{
    free(server);
}

// Function to return a new HTTPResponse struct with all of the fields set to default empty values and the date field set to the current timestamp
struct HTTPResponse createNewHTTPResponseStruct()
{
    // Create response struct
    struct HTTPResponse response;
    // Set status code to zero (an empty invalid status code)
    response.statusCode = 0;

    // Set content length to -1 to indicate it is unset
    response.contentLength = -1;

    // Set the date value to the current timestamp
    response.date = time(NULL);

    // Set all strings to NULL
    response.statusMessage = NULL;
    response.contentType = NULL;
    response.content = NULL;
    response.location = NULL;

    // Return the response struct
    return response;
}

// Function to get the HTTP status message for a given status code
char *getHTTPStatusMessage(int code)
{
    // If the code is not valid then return "Unknown Status Code"
    if (code < 100 || code >= 600)
    {
        return "Unknown Status Code";
    }

    // If the code is not implemented then return "Unknown Status Code"
    if (HTTP_Status_Codes[code] == NULL)
    {
        return "Unknown Status Code";
    }

    // Access the status message from the HTTP_Status_Codes array and return it
    return (char *)HTTP_Status_Codes[code];
}

// Converts a given time_t date into a HTTP header compatible string
char *dateToHTTPString(time_t date)
{
    // Define buffer to hold the string representation of the date
    size_t bufferSize = 128;
    char *buffer = malloc(sizeof(char) * bufferSize);

    // Convert time_t to struct tm for use in the strftime function
    struct tm *time = gmtime(&date);

    // Convert tm struct into string using strftime with correct format string
    strftime(buffer, bufferSize, "%a, %d, %b, %Y, %H:%M:%S GMT", time);

    // Return the string
    return buffer;
}

// Function that converts a given HTTPResponse struct into a full HTTP response string
char *createResponseStringFromStruct(struct HTTPResponse response, int *responseSize)
{
    // Build the status line of the HTTP response
    // Allocate the required memory for the status line
    char *statusLine = (char *)malloc(sizeof(char) * (strlen(response.statusMessage) + 100));
    // Using sprintf to build the status line using the HTTP version, status code and message
    sprintf(statusLine, "%s %d %s\r\n", "HTTP/1.1", response.statusCode, response.statusMessage ? response.statusMessage : getHTTPStatusMessage(response.statusCode));

    // Build the headers section of the HTTP response

    // Define max length for the response headers
    const unsigned short int MAXHEADERLENGTH = 4096;

    // Allocate the required memory for the headers
    char *headers = (char *)malloc(sizeof(char) * MAXHEADERLENGTH);

    // Fixed headers that are added to every response
    const char FixedHeaders[] = "Cache-Control: no-store\r\nX-Content-Type-Options: nosniff\r\nStrict-Transport-Security: max-age=63072000; includeSubDomains;";

    // If no content length is provided, calculate the content length from the content of the response
    int contentLength = 0;
    if (response.contentLength == -1)
    {
        // If the response has content set the length to the length of the content otherwise, leave the content length at its default value of 0
        if (response.content)
        {
            contentLength = strlen(response.content);
        }
    }
    else
    {
        // Use the provided content length value if available
        contentLength = response.contentLength;
    }

    // Using snprintf to build the headers with the content type and the string form of the date
    snprintf(headers, MAXHEADERLENGTH, "%s%s%s%s%s%sContent-Length: %d\r\nDate: %s\r\n%s\r\n\r\n",
             response.location ? "Location: " : "",
             response.location ? response.location : "",
             response.location ? "\r\n" : "",
             response.contentType ? "Content-Type: " : "",
             response.contentType ? response.contentType : "",
             response.contentType ? "\r\n" : "",
             contentLength, dateToHTTPString(response.date), FixedHeaders);

    // Build the final response string using the status line, headers and content

    // Calculate the total length of the response string and null terminator at the end of the string
    *responseSize = strlen(statusLine) + strlen(headers) + (response.content ? contentLength : 0);

    // Allocate the required memory for the response string
    char *responseString = (char *)malloc(sizeof(char) * ((*responseSize) + 1));

    // Copy the status line into the response string
    memcpy(responseString, statusLine, strlen(statusLine));

    // Copy the headers into the response string after the status line
    memcpy(responseString + strlen(statusLine), headers, strlen(headers));

    // Copy the content into the response string after the headers
    memcpy(responseString + strlen(statusLine) + strlen(headers), response.content ? response.content : "", response.content ? contentLength : 0);

    // Add a null terminator to the end of the response string
    responseString[*responseSize] = '\0';

    // Return the reponse string
    return responseString;
}

// Sets up the certificate and private key for a given SSL session
void setupCertificate(SSL *ssl)
{
    // Setup certificate
    SSL_use_certificate_chain_file(ssl, "cert.pem");
    // Setup private key
    SSL_use_PrivateKey_file(ssl, "key.pem", SSL_FILETYPE_PEM);
}

// Converts a given relative file path from a request into an absolute file path
char *getAbsolutePathFromRelative(char *relativePath, int *problem)
{
    int lastPathSeperatorIndex = -1;
    int lastFullstopIndex = -1;

    // Stores whether a querty string is found and its position
    bool queryFound = false;
    int queryPos = 0;
    // Iterate backwards through the file path to determine if the request is a file or a directory and if it contains a query string
    for (int i = strlen(relativePath) - 1; i >= 0; i--)
    {
        // Find the last path seperator index
        if (relativePath[i] == '/')
        {
            // If the last path seperator has not be found yet set it to the current index
            if (lastPathSeperatorIndex == -1)
            {
                lastPathSeperatorIndex = i;
            }
            // If the last path seperator has already been found then break the loop as the path is a directory
            else
            {
                break;
            }
        }
        // Finds the last full stop index and checks it is not for a directory before storing the value
        else if (relativePath[i] == '.')
        {
            // If the full stop is at the start of the path then the path is a directory so break the loop
            if (i == 0)
            {
                break;
            }
            else if (relativePath[i - 1] == ' ')
            {
                break;
            }
            else
            {
                // If the full stop is directly after a path seperator then it is for a directory so break the loop
                if (relativePath[i - 1] == '/')
                {
                    break;
                }
            }
            // If the full stop is not for a directory then store the index and break the loop
            lastFullstopIndex = i;
            break;
        }
        // Check if a query string is found and store its position so that index.html can be added if needed
        else if (relativePath[i] == '?')
        {
            queryFound = true;
            queryPos = i;
            break;
        }
    }

    // Create a temporary copy of the relative path to modify if the path is a directory
    char *tempPath = strdup(relativePath);

    // If there is no full stop in the file path then it is a directory so index.html should be appended to the path
    bool addIndexHTML = (lastFullstopIndex == -1);
    if (addIndexHTML)
    {
        // If a query string is found then modify the buffer to remove it and add a trailing slash so index.html can be appended
        if (queryFound)
        {
            // Remove the query string by replacing the start of it with a trailing slash and null terminator
            tempPath[queryPos] = '/';
            tempPath[queryPos + 1] = '\0';
        }
        // If there is no trailing slash, the user should be redirected to the same URL with one added
        else if (tempPath[strlen(tempPath) - 1] != '/')
        {
            // Set the problem variable to NO_TRAILING_SLASH to show that a redirect should be sent
            *problem = NO_TRAILING_SLASH;

            // Return null as there is no valid file path
            return NULL;
        }
    }

    // Calculate the total path length summing the CWD length, relative path length and optionally the index.html length
    int pathLength = CWD_len + strlen(tempPath) + 10 * addIndexHTML;

    // Create the full path

    // Allocate the required memory for the full path
    char *path = malloc(sizeof(char) * (1 + pathLength));

    // Copy the CWD to the path then add the relative path from the request
    strncpy(path, CWD, CWD_len);
    strncpy(path + CWD_len, tempPath, strlen(tempPath));

    // If index.html needs to be added then append it to the end of the path
    if (addIndexHTML)
    {
        strncpy(path + CWD_len + strlen(tempPath), "index.html", 10);
    }

    // Add a null terminator to the end of the path to indicate the end of the string
    path[pathLength] = '\0';

    // Return the final path
    return path;
}

// Function to get the value of the host name header from a given request buffer
char *getHostNameFromBuffer(char *buffer)
{
    // Find the start of the host header using strstr
    char *hostStart = strstr(buffer, "Host: ");

    // If the host header was not found, return NULL
    if (!hostStart)
    {
        return NULL;
    }

    // Find the end of the host header using strstr to find the newline characters
    char *hostEnd = strstr(hostStart, "\r\n");

    // If the end of the header was not found return NULL
    if (!hostEnd)
    {
        return NULL;
    }

    // Go past the host name
    hostStart += 6;

    // Calculate length of host value using pointers
    size_t hostLength = hostEnd - hostStart;

    // Declare the host name string and allocate memory for it
    char *hostName = malloc(sizeof(char) * (hostLength + 1));

    // Copy the host name into the variable
    memcpy(hostName, hostStart, hostLength);

    // Set the null terminator at the end of the string
    hostName[hostLength] = '\0';

    return hostName;
}

// Sends a given HTTP response object to a given SSL session
void sendHTTPResponse(SSL *ssl, struct HTTPResponse response)
{
    // Declare response size
    int responseSize = 0;

    // Call string creation function using the response struct and a pointer to the size variable
    char *responseBuffer = createResponseStringFromStruct(response, &responseSize);

    // Write the data to the SSL session
    SSL_write(ssl, responseBuffer, responseSize);
}

// Redirects a given SSL session to the given URL with a 301 Moved Permanently code
void redirect(char *redirectURL, SSL *ssl)
{
    // Create a new response struct
    struct HTTPResponse response = createNewHTTPResponseStruct();

    // Set the content length to zero as there is no body/content in a redirect
    response.contentLength = 0;

    // Set the status to permanent redirect
    response.statusCode = 301;
    response.statusMessage = getHTTPStatusMessage(301);

    // Set the content type to the default plain text (best practice)
    response.contentType = "text/plain";

    // Set the location header to the redirect URL
    response.location = redirectURL;

    sendHTTPResponse(ssl, response);
}

// Redirects a given SSL session to the same URL with a trailing slash added
void trailingSlashRedirect(struct HTTPRequest request)
{
    // Store the length of the relative path
    int relPathLength = strlen(request.relativePath);

    // Allocate the required memory for the new URL with a trailing slash
    char *fixedURL = malloc(sizeof(char) * (2 + relPathLength));

    // Copy the URL from the request into the new URL buffer
    memcpy(fixedURL, request.relativePath, relPathLength);

    // Add the trailing slash to the end of the URL
    fixedURL[relPathLength] = '/';

    // Add a null terminator to the end of the URL to indicate the end of the string
    fixedURL[relPathLength + 1] = '\0';

    // Send the redirect response to the client with the fixed URL
    redirect(fixedURL, request.sslSession);
}

// Sends a code 400 bad request response to the client
void send400Response(struct HTTPRequest request)
{
    // Create a new response struct
    struct HTTPResponse response = createNewHTTPResponseStruct();

    // Set the content length to zero as there is no body/content in a redirect
    response.contentLength = 0;

    // Set the status to bad request
    response.statusCode = 400;
    response.statusMessage = getHTTPStatusMessage(301);

    // Set the content type to the default plain text (best practice)
    response.contentType = "text/plain";

    sendHTTPResponse(request.sslSession, response);
}

// Finds the current working directory and stores it in a variable
void findCWD()
{
    // Get the current working directory and store it in the global CWD variable
    getcwd(CWD, sizeof(CWD));

    // Append the static path (public) to the CWD variable so it contains the full path to the frontend files
    strncpy(CWD + strlen(CWD), STATICPATH, strlen(STATICPATH));

    // Store the length of the CWD variable in a CWD_len global
    CWD_len = strlen(CWD);
}

// Sends an empty OK response to the client
void sendEmptyResponse(SSL *ssl)
{
    // Create the new response struct
    struct HTTPResponse response = createNewHTTPResponseStruct();

    // Set the status to 200 OK
    response.statusCode = 200;
    response.statusMessage = getHTTPStatusMessage(200);

    // Set the content type to plain text
    response.contentType = "text/plain";

    // Use the send function to send the empty response
    sendHTTPResponse(ssl, response);
}

// Sends a 404 content not found response to the client
void send404Code(SSL *ssl)
{
    // Response header for the 404 content not found response
    char *metadata = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";

    // Create buffer for the entire HTTP response
    char response_data[1024] = {0};

    // Copy the response header into the buffer
    memcpy(response_data, metadata, strlen(metadata));

    // Send the HTTP response to the client
    SSL_write(ssl, response_data, 1024);
}

// Stops a given SSL session, closes the connection and frees the required memory
void stopConnection(SSL *ssl, int clientfd)
{
    // Shutdown the SSL session
    SSL_shutdown(ssl);

    // Close the client connection
    close(clientfd);

    // Free memory from the SSL session
    SSL_free(ssl);
}

// Sends a 404 content not found response to the client
void send404Header(SSL *ssl)
{
    char *metadata = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n";
    char response_data[1024] = {0};
    memcpy(response_data, metadata, strlen(metadata));
    SSL_write(ssl, response_data, 1024);
}

// Function to get the HTTP request method from a given request buffer
enum HTTPRequestMethod getMethodFromRequest(char *buffer)
{
    // Check for GET request
    if (strncmp(buffer, "GET ", 4) == 0)
    {
        return GET;
    }
    // Check for HEAD request
    else if (strncmp(buffer, "HEAD ", 5) == 0)
    {
        return HEAD;
    }
    // Check for POST request
    else if (strncmp(buffer, "POST ", 5) == 0)
    {
        return POST;
    }
    // Check for DELETE request
    else if (strncmp(buffer, "DELETE ", 7) == 0)
    {
        return DELETE;
    }

    // Return GET by default if the method is not recognised
    return GET;
}

// Function to extract the relative path from a given request buffer
char *getRelativePathFromRequest(char *buffer)
{
    // Find the first space character in the request, this is the start of the file path
    int firstSpace = -1;
    for (int i = 0; i < strlen(buffer); i++)
    {
        if (buffer[i] == ' ')
        {
            firstSpace = i + 1;
            break;
        }
    }

    // Check that a space character was found
    if (firstSpace == -1)
    {
        return NULL;
    }

    // Find the second space character in the request, this is the end of the file path
    int secondSpace = -1;
    for (int i = firstSpace + 1; i < strlen(buffer); i++)
    {
        if (buffer[i] == ' ')
        {
            secondSpace = i;
            break;
        }
    }

    // Check that the second space character was found
    if (secondSpace == -1)
    {
        return NULL;
    }

    // Calculate the length of the file path
    int relativePathLength = secondSpace - firstSpace;

    // Allocate the required memory for the relative path including a null terminator
    char *relativePath = malloc(sizeof(char) * (relativePathLength + 1));

    // Copy the file path from the request into the relative path buffer
    strncpy(relativePath, buffer + firstSpace, relativePathLength);

    // Add the null terminator to the end of the relative path string
    relativePath[relativePathLength] = '\0';

    // Return the relative path
    return relativePath;
}

// Function to extract the request body from a given request buffer
char *getRequestBodyFromBuffer(char *buffer)
{

    // Find the end of the headers in the request
    char *headerEnd = strstr(buffer, "\r\n\r\n");

    // Check if the end of the headers was found, this should be present in a valid HTTP request
    if (headerEnd == NULL)
    {
        return NULL;
    }

    // Calculate the length of the request body
    int bodyLength = strlen(headerEnd) - 4;

    // Check if there is no body in the request
    if (bodyLength <= 0)
    {
        return NULL;
    }

    // Allocate the required memory for the request body including a null terminator
    char *body = malloc(sizeof(char) * (bodyLength + 1));

    // Copy the body from the request buffer into the body buffer
    memcpy(body, headerEnd + 4, bodyLength);

    // A null terminator to the end of the body string
    body[bodyLength] = '\0';

    // Return the request body
    return body;
}

// Function that takes a request buffer and converts it into a HTTPRequest struct
struct HTTPRequest createHTTPRequestStructFromBuffer(char *buffer, SSL *sslSession, int *problem)
{
    struct HTTPRequest request;

    // Get the HTTP method from the request
    request.method = getMethodFromRequest(buffer);

    // Get the relative file path from the request
    request.relativePath = getRelativePathFromRequest(buffer);

    // Get the absolute file path from the relative path
    request.absolutePath = getAbsolutePathFromRelative(request.relativePath, problem);

    // Get the request body from the request buffer
    request.body = getRequestBodyFromBuffer(buffer);

    // Set the SSL session for the request
    request.sslSession = sslSession;

    // Return the HTTPRequest struct instance
    return request;
}

// Main function for running the HTTPS webserver
void *Run_HTTPS_Server()
{
    // Create the listener socket for the HTTPS server
    int socketfd = createListenerSocket(HTTPSPORT, MAXQUEUED);
    printf("Started HTTPS server\n");

    // Run the webserver loop
    while (true)
    {
        // Accept a client connection and store its file descriptor
        int client_fd = accept(socketfd, NULL, NULL);

        // Verify that the client connection was valid, skip this loop iteration if not
        if (client_fd == -1)
        {
            printf("An error occurred when attempting to accept a request, errno: %d\n", errno);
            continue;
        }

        // Setup SSL for the current client request

        // Create SSL context
        SSL_CTX *ssl_ctx = SSL_CTX_new(TLS_server_method());
        // Create SSL session from context
        SSL *ssl_session = SSL_new(ssl_ctx);

        // Bind the current client connection to the SSL session
        SSL_set_fd(ssl_session, client_fd);
        // Setup the SSL certificates and private keys
        setupCertificate(ssl_session);
        // Attempt to accept the SSL connection
        SSL_accept(ssl_session);

        // Create 4KB buffer for storing HTTPS requests
        char bytesBuffer[4096] = {0};
        // Read request up to 4096 bytes (leaving room for null terminator)
        SSL_read(ssl_session, bytesBuffer, 4095);

        // If the request contained no data, send an empty response
        if (bytesBuffer[0] == '\0')
        {
            sendEmptyResponse(ssl_session);
            stopConnection(ssl_session, client_fd);
            continue;
        }

        // Convert the request buffer into a request struct
        int problem = -1;
        struct HTTPRequest request = createHTTPRequestStructFromBuffer(bytesBuffer, ssl_session, &problem);

        // If a problem occurs when extracting the file path send a redirect if the trailing slash is missing, otherwise close the connection
        if (problem != -1)
        {
            switch (problem)
            {
                // If there is no trailing slash, redirect to the same URL with a trailing slash and close this connection
            case NO_TRAILING_SLASH:
                trailingSlashRedirect(request);
                stopConnection(ssl_session, client_fd);
                continue;
                break;

                // If some other problem occurs, send a 400 bad response code and close the connection
            default:
                send400Response(request);
                stopConnection(ssl_session, client_fd);
                continue;
                break;
            }
        }

        // Create the response struct
        struct HTTPResponse response = createNewHTTPResponseStruct();

        // Get the content type from the file path
        response.contentType = getTypeFromPath(request.absolutePath);

        // Reset the problem variable to -1 before reading file content
        problem = -1;

        // Read the file content from the requested file path
        response.content = ReadDataFromFile(request.absolutePath, (size_t *)&response.contentLength, &problem);

        // If the request is a head
        if (request.method == HEAD)
        {
            response.content = NULL;
        }

        // If there was a problem reading the file content then send a 404 response and close the connection
        if (problem != -1)
        {
            // Set the status code to 404
            response.statusCode = 404;

            if (strncmp(response.contentType, "text/html", strlen(response.contentType)) == 0)
            {

                // Reset problem to -1 before getting relative path
                problem = -1;

                // Get the absolute path for the 404 page
                char *path404 = getAbsolutePathFromRelative("/404/", &problem);

                // Create a second problem variable for reading the file content
                int problem2 = -1;

                // Read the content of the 404 file
                response.content = ReadDataFromFile(path404, (size_t *)&response.contentLength, &problem2);

                // If there was a problem getting 404 page use HTTP status code 500 internal server error and set content length to 0
                if (problem != -1 || problem2 != -1)
                {
                    // Set status code
                    response.statusCode = 500;

                    // Set content length to zero as the content is not valid
                    response.contentLength = 0;
                }
            }
            // If the request is for a file instead of a page, do not send any content
            else
            {
                response.contentLength = 0;
                response.content = NULL;
            }
        }

        // If the status code isn't already set, set it to 200 OK
        if (response.statusCode == 0)
        {
            response.statusCode = 200;
        }

        // Get the status message for the current status code
        response.statusMessage = getHTTPStatusMessage(response.statusCode);

        // Create the response string
        int responseSize = 0;
        char *responseBuffer = createResponseStringFromStruct(response, &responseSize);

        // Send the HTTP response to the client
        SSL_write(ssl_session, responseBuffer, responseSize);

        // Stop the connection
        stopConnection(ssl_session, client_fd);
    }
}

// For redirecting HTTP to HTTPS
void *Run_HTTP_Server()
{
    // Create listener socket on the HTTP port
    int socketfd = createListenerSocket(HTTPPORT, MAXQUEUED);
    printf("Started HTTP server\n");

    // Declare constant buffer size
    const int BUFFER_SIZE = 4096;

    while (true)
    {
        // Accept a client connection and store its file descriptor
        int client_fd = accept(socketfd, NULL, NULL);
        // Verify that the client connection was accepted successfully
        if (client_fd == -1)
        {
            printf("An error occurred when attempting to accept a request, errno: %d\n", errno);
        }

        // Create buffer
        char buffer[BUFFER_SIZE];

        // Read data from request into buffer and store size of request in bytesReceived
        int bytesReceived = recv(client_fd, buffer, BUFFER_SIZE, 0);

        // If no bytes were received, close the connection
        if (bytesReceived <= 0)
        {
            close(client_fd);
            continue;
        }

        // Get the host name from the request buffer
        char *hostName = getHostNameFromBuffer(buffer);

        // Validate host name
        if (hostName == NULL)
        {
            close(client_fd);
            continue;
        }

        // Create a new HTTP response struct
        struct HTTPResponse response = createNewHTTPResponseStruct();

        // Set the response status code and message
        response.statusCode = 301;
        response.statusMessage = getHTTPStatusMessage(301);

        // Set the location header
        response.location = malloc(strlen(hostName) + 10);
        sprintf(response.location, "https://%s/", hostName);

        // Create the response string and store its size
        int responseSize;
        char *responseString = createResponseStringFromStruct(response, &responseSize);

        // Send the HTTP redirect response to the client and close the connection
        send(client_fd, responseString, responseSize, 0);
        close(client_fd);

        // Free allocated memory
        free(hostName);
        free(responseString);
    }
}

// Initalises and starts both the HTTP and HTTPS webserver
int mainOld(void)
{

    // Set the current working directory global variable
    findCWD();

    // Setup content type data structures
    setupContentTypes();

    // Create threads for http and https servers to run on seperately
    pthread_t HTTP_thread,
        HTTPS_thread;

    // Start the HTTP and HTTPS servers on their respective threads
    pthread_create(&HTTP_thread, NULL, Run_HTTP_Server, NULL);
    pthread_create(&HTTPS_thread, NULL, Run_HTTPS_Server, NULL);

    // Wait for both the webserver threads to finish before stopping the server
    pthread_join(HTTP_thread, NULL);
    pthread_join(HTTPS_thread, NULL);

    return 0;
}