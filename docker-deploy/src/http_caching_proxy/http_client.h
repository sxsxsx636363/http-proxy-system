#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

#include "http_request.h"
#include "http_response.h"
#include "socket_wrapper.h"

typedef std::shared_ptr<HttpRequest> HttpRequestPtr;
typedef std::shared_ptr<HttpResponse> HttpResponsePtr;
typedef std::shared_ptr<SocketWrapper> socketPtr;

class HttpClient {
public:
    HttpClient() {}
    ~HttpClient() {}
    socketPtr init(HttpRequestPtr req);
    bool sendRequest(HttpRequestPtr req);
    HttpResponsePtr recvResponse();

private:
    socketPtr sockFd;
    int getSockVal(socketPtr s) { return s.get()->get(); }
};

#endif