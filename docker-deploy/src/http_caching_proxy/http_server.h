#ifndef HTTP_SERVER
#define HTTP_SERVER

#include <memory>

#include "http_request.h"
#include "http_response.h"
#include "http_client.h"
#include "socket_wrapper.h"
#include "cache.h"
#include "agent.h"
typedef std::shared_ptr<HttpRequest> HttpRequestPtr;
typedef std::shared_ptr<HttpResponse> HttpResponsePtr;
typedef std::shared_ptr<SocketWrapper> socketPtr;

class HttpServer {
public:
    HttpServer();
    ~HttpServer();
    void startServer();
    void process(socketPtr newSockFd);
    
private:
    void init();
    bool sendResponse(HttpResponsePtr resp, socketPtr newSockFd);
    HttpRequestPtr recvRequest(socketPtr newSockFd);
    void process_connect(HttpRequestPtr req, socketPtr newSockFd);
    void process_chunked(HttpClient& client, HttpResponsePtr resp, socketPtr newSockFd);
    
    socketPtr sockFd;
    int getSockVal(socketPtr s) { return s.get()->get(); }

};

#endif